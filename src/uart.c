/**
 * @file   uart.c
 *
 * @brief  Interrupt and Console I/O
 *
 * @date   March 3rd
 *
 * @author Yuhong Yao
 */

#include <unistd.h>
#include <rcc.h>
#include <uart.h>
#include <uart_polling.h>
#include <nvic.h>
#include <gpio.h>

#define UNUSED __attribute__((unused))

#define BUFFER_SIZE (16)

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};

/** @brief Base address for UART2 */
#define UART2_BASE      (struct uart_reg_map *) 0x40004400

/** @brief Enable Bit for UART Config register */
#define UART_EN         (1 << 13)

/** @brief Pre calculated UARTDIV value for desired band rate of 115200 bps by default */
#define UARTDIV         0x8B

/** @brief Enable Bit for Transmitter */
#define UART_TE         (1 << 3)

/** @brief Enable Bit for Receiver */
#define UART_RE         (1 << 2)

/** @brief Transmit data register empty */
#define UART_SR_TXE     (1 << 7)

/** @brief Read data registter not empty */
#define UART_SR_RXNE    (1 << 5)

#define UART_CR1_RXNEIE (1 << 5)
#define UART_CR1_TXEIE  (1 << 7)

typedef struct {
    unsigned char buffer[BUFFER_SIZE];
    volatile unsigned int head;
    volatile unsigned int tail;
} RingBuffer;

typedef struct {
    RingBuffer txBuffer;
    RingBuffer rxBuffer;
} UARTIO;

UARTIO uartIO;  // global variable for managing uart io

// initialize ring buffer as empty
void RingBuffer_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
}

int RingBuffer_isEmpty(RingBuffer *rb) {
    return rb->head == rb->tail;
}

int RingBuffer_isFull(RingBuffer *rb) {
    return (((rb->tail + 1) & (BUFFER_SIZE - 1)) == rb->head); 
}

int RingBuffer_Write(RingBuffer *rb, unsigned char data) {
    if (RingBuffer_isFull(rb)) {
        return -1; // buffer is full
    }
    // write at the tail
    rb->buffer[rb->tail] = data;
    rb->tail = (rb->tail + 1) & (BUFFER_SIZE - 1);
    return 0;
}

int RingBuffer_Read(RingBuffer *rb, unsigned char *data) {
    if (RingBuffer_isEmpty(rb)) {
        return -1;
    }
    *data = rb->buffer[rb->head];
    rb->head = (rb->head + 1) & (BUFFER_SIZE - 1);
    return 0;
}

void uart_init(UNUSED int baud) {
    //init ring buffer
    RingBuffer_init(&uartIO.txBuffer);
    RingBuffer_init(&uartIO.rxBuffer);

    if (baud == 0) {
        return;
    }
    struct uart_reg_map *uart = UART2_BASE;
    // Reset and Clock Control
    struct rcc_reg_map *rcc = RCC_BASE;
    rcc->apb1_enr |= UART_CLKEN;
    // GPIO Pins
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);        /* PA_2 for TX line UART2 */
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);       /* PA_2 for RX line UART2 */
    
    // Initialize UART to the desired Baud Rate
    *(uint16_t*)&uart->BRR = (uint16_t)UARTDIV;
    // UART Control Registers
    nvic_irq(38, IRQ_ENABLE);
    uart->CR1 |= (UART_TE | UART_RE | UART_EN | UART_CR1_RXNEIE);
    return;
}

int uart_put_byte(UNUSED char c) {
    struct uart_reg_map *uart = UART2_BASE;
    if (RingBuffer_isFull(&uartIO.txBuffer)) {
        return -1;
    }
    RingBuffer_Write(&uartIO.txBuffer, (unsigned char)c);
    uart->CR1 |= UART_CR1_TXEIE;
    return 0;
}

int uart_get_byte(UNUSED char *c) {
    if (RingBuffer_isEmpty(&uartIO.rxBuffer)) {
        return -1;
    }
    unsigned char data;
    if (RingBuffer_Read(&uartIO.rxBuffer, &data) == 0) {
        *c = data;
        return 0;
    }
    return -1;
}

int uart_write(UNUSED int file, UNUSED char *ptr, UNUSED int len) {
    struct uart_reg_map *uart = UART2_BASE;
    if (file != STDOUT_FILENO) {
        return -1;
    }

    for (int i = 0; i < len; i++) {
        RingBuffer_Write(&uartIO.txBuffer, ptr[i]);
        uart->CR1 |= UART_CR1_TXEIE;
    }
    return len;
}

int uart_read(UNUSED int file, UNUSED char *ptr, UNUSED int len) {
    if (file != STDIN_FILENO) {
        return -1;
    }
    
    int byteRead = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c;
        RingBuffer_Read(&uartIO.rxBuffer, &c);

        if (c == 4) {
            break;
        } else if (c == '\b') {
            if (byteRead > 0) {
                byteRead--;
                uart_write(STDOUT_FILENO, "\b \b", 3);
            }
        } else if (c == '\r') {
            ptr[byteRead++] = '\n';
            uart_write(STDOUT_FILENO, "\r\n", 2);
            break;
        } else if (c == '\n') {
            ptr[byteRead++] = '\n';
            uart_write(STDOUT_FILENO, "\n", 1);
            break;

        } else {
            ptr[byteRead++] = c;
            uart_write(STDOUT_FILENO, (char *)&c, 1);
        }
    }
    return byteRead;
}

void uart_irq_handler() {
    struct uart_reg_map *uart = UART2_BASE;
    int bytesHandled = 0; // To keep track of the bytes sent or received

    // Handle Transmission if TXE is set and the TX buffer is not empty
    while ((uart->SR & UART_SR_TXE) && (!RingBuffer_isEmpty(&uartIO.txBuffer)) && (bytesHandled < 16)) {
        unsigned char data;
        if (RingBuffer_Read(&uartIO.txBuffer, &data) == 0) {
            uart->DR = data; // Send data
            bytesHandled++;
        }
    }
    // If the buffer is empty after sending data, disable TX interrupt
    if (RingBuffer_isEmpty(&uartIO.txBuffer)) {
        uart->CR1 &= ~UART_CR1_TXEIE;
    }

    // Reset byte handling counter for receiving part
    bytesHandled = 0;

    // Handle Reception if RXNE is set
    while ((uart->SR & UART_SR_RXNE) && (bytesHandled < 16)) {
        unsigned char data = uart->DR; // Reading DR clears the RXNE flag
        if (!RingBuffer_isFull(&uartIO.rxBuffer)) {
            RingBuffer_Write(&uartIO.rxBuffer, data);
            bytesHandled++;
        } 
    }

    nvic_clear_pending(38);
}
