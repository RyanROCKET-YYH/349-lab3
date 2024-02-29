#include <gpio.h>
#include <i2c.h>
#include <printk.h>
#include <uart_polling.h>
#include <unistd.h>
#include <lcd_driver.h>
#include <keypad_driver.h>

// define the passcode's length
#define PASSCODE_LENGTH   (4)

/*
 * lazy_delay():
 * To make a delay.
*/
void lazy_delay(unsigned int milliseconds) {
  volatile unsigned int i, j;
  for (i = 0; i < milliseconds; i++) {
    for (j = 0; j < 10000; j++) {
      __asm("NOP");
    }
  }
}

/*
 * strncmp():
 * To compare char array s1 and s2
*/
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (n == (size_t)-1) {
        return 0;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/*
 * clear_passcode():
 * To clear the buffer of the passcode
*/
void clear_passcode(char *buffer, size_t length) {
  for (size_t i = 0; i < length; i++) {
    buffer[i] = 0;
  }
}

/*
 * lock_system():
 * To lock the system, and print Locked in minicom
*/
void lock_system(char *passcode, uint8_t *is_locked, uint8_t *index, uint8_t *row, uint8_t *col) {
  // clear the passcode buffer
  clear_passcode(passcode, PASSCODE_LENGTH);
  *is_locked = 1;
  *index = 0;
  // set GPIO
  gpio_set(GPIO_B, 5);
  gpio_clr(GPIO_A, 10);

  // clear the lcd
  lcd_clear();
  printk("Locked, ENTER PASSCODE:\n");
  *row = 0;
  *col = 0;
}

/*
 * enter_button():
 * To check if passcode is correct. 
*/
void enter_button(char *passcode, uint8_t *is_locked, uint8_t *index, const char *correct_passcode, uint8_t *row, uint8_t *col) {
  // check if passcode is correct
  if (*index == PASSCODE_LENGTH && passcode[0] == '#' && strncmp(passcode, correct_passcode, PASSCODE_LENGTH) == 0) {
    *is_locked = 0;
    // set GPIO
    gpio_clr(GPIO_B, 5);
    gpio_set(GPIO_A, 10);
    printk("Unlocked\n");
  } else {
    printk("Incorrect Passcode, try again!\n");
  }

  // clear the lcd
  lcd_clear();

  // clear the passcode buffer
  clear_passcode(passcode, PASSCODE_LENGTH);
  *index = 0;
  *row = 0;
  *col = 0;
}

/*
 * key_display():
 * set the lcd cursor position.
*/
void key_display(char key, uint8_t *row, uint8_t *col) {
    if (*col >= 16) {
        *col = 0;
        if (*row == 0) {
            *row = 1;
        } else {
            lcd_clear();
            *row = 0; // Reset to first line
        }
    }

    lcd_set_cursor(*row, *col);
    char display[2] = {key, '\0'}; // Prepare the string for display
    lcd_print(display); // Show the key pressed

    (*col)++; // Move cursor position forward
}

/*
 * main():
 * the main function to implement the lock system.
*/
int main() {
  // initialize the uart and keypad
  uart_polling_init(115200);
  keypad_init();

  // set GPIO
  // BLUE LED (D2)
  gpio_init(GPIO_A, 10, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  // RED LED (D4)
  gpio_init(GPIO_B, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  // "ENTER" BUTTON (D6)
  gpio_init(GPIO_B, 10, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
  // "LOCK" BUTTON (B1)
  gpio_init(GPIO_C, 13, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);

  // initialize the i2c_master and lcd_driver
//   i2c_master_init(80);
//   lcd_driver_init();
//   lcd_clear();

//   // set variables for passcode, lock_systems, and lcd cursor
//   char passcode[PASSCODE_LENGTH] = {0};
//   uint8_t index = 0;
//   uint8_t is_locked = 1;
//   const char correct_passcode[PASSCODE_LENGTH] = {'#', '3', '4', '9'}; // correct passcode is "#349"
//   uint8_t row = 0; //lcd cursor
//   uint8_t col = 0; //lcd cursor

  printk("Enter 'Start' to begin:\n");
//   uint8_t start = 0;
//   // make user type in "Start" to enter the passcode
//   while(!start) {
//     char input[5] = {0}; // buffer for start
//     for (int i = 0; i < 5; i++) {
//       input[i] = uart_polling_get_byte();
//       uart_polling_put_byte(input[i]);
//     }
//     input[5] = '\0';
//     if (strncmp(input, "Start", sizeof(input)) == 0) {
//       start = 1;
//       printk("\n What's your password?\n");
//     } else {
//       start = 0;
//       printk("\n Enter 'Start' to begin:\n");
//     }
//   }

//   // set RED LED (D4)
//   gpio_set(GPIO_B, 5);


//   while(1) {
//     // check "LOCK" BUTTON (B1)
//     if (!gpio_read(GPIO_C, 13)) {
//       lock_system(passcode, &is_locked, &index, &row, &col);
//       lazy_delay(10);
//     }

//     // check "ENTER" BUTTON (D6)
//     if (!gpio_read(GPIO_B, 10) && is_locked) {
//       enter_button(passcode, &is_locked, &index, correct_passcode, &row, &col);
//       lazy_delay(10);
//     }

//     // read passcode from keypad
//     char key = keypad_read();
//     if (is_locked) {
//       if (key != '\0') {
//         passcode[index++] = key;
//         key_display(key, &row, &col);
//         lazy_delay(2); // prevent double press
//       }
//     } else {
//       if (key != '\0') {
//         key_display(key, &row, &col);
//         lazy_delay(2); // prevent double press
//       }
//       if (!gpio_read(GPIO_B, 10)) {
//         lcd_clear();
//         lcd_set_cursor(0,0);
//       }
//     }
//   }
  return 0;
}
