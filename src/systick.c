/**
* @file
*
* @brief
*
* @date
*
* @author
*/

#include <unistd.h>
#include <systick.h>
#include <printk.h>

#define UNUSED __attribute__((unused))

/** @brief The systick(STK) timer register map. */
struct stk_reg_map {
    volatile uint32_t CTRL;      /**<  control and status register (STK_CTRL) */
    volatile uint32_t LOAD;      /**<  SysTick reload value register (STK_LOAD) */
    volatile uint32_t VAL;     /**<  SysTick current value register (STK_VAL) */
    volatile uint32_t CALIB;     /**<  SysTick calibration value register (STK_CALIB) */
};

/** @brief Base Address of STK */
#define STK_BASE   (struct stk_reg_map *) 0xE000E010

/** @brief STK CTRL: Counter ENABLE bit */
#define STK_CTRL_EN 1 
/** @brief STK CTRL: TICKINT bit */
#define STK_CTRL_TICKINT (1 << 1)
/** @brief STK CTRL: CLKSOURCE bit */
#define STK_CTRL_CLKSOURCE (1 << 2)
// count the glocal tick
volatile uint32_t g_tick_count; // set in systick.h

void systick_init() {
    
    struct stk_reg_map* stk = STK_BASE;

    // Frequency: 16MHz
    stk->LOAD = 15999;

    // When ENABLE is set to 1, the counter loads the RELOAD value from the LOAD register and then counts down
    stk->CTRL |= STK_CTRL_EN;
    stk->CTRL |= STK_CTRL_TICKINT;
    stk->CTRL |= STK_CTRL_CLKSOURCE;

}

/**
* systick_delay():
* @brief delays the processor for ticks milliseconds
*
*/
void systick_delay(uint32_t ticks) {
    uint32_t start = g_tick_count;
    // set delay
    while ((g_tick_count - start) < ticks) {
        // wait
    }
}

uint32_t systick_get_ticks() {
    return g_tick_count;
    // return -1;
}

void systick_c_handler() {

    systick_init();

    // Increment tick counter in SysTick ISR
    g_tick_count++;
    printk("\ng_tick_count = %d\n", g_tick_count);

    uint32_t test_get_ticks = systick_get_ticks();
    printk("\nsystick_get_ticks = %d\n", test_get_ticks);

    // set delay: 500 milliseconds
    systick_delay(500);
    printk("\nset delay: 500 millisecond\n");
    test_get_ticks = systick_get_ticks();
    printk("\nsystick_get_ticks = %d\n", test_get_ticks);
}
