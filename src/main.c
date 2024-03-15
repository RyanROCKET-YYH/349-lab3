#include <gpio.h>
#include <i2c.h>
#include <printk.h>
#include <uart_polling.h>
#include <unistd.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
#include <systick.h>
#include <string.h>
#include <uart.h>
#include <timer.h>
#include <servo.h>
#include <stdlib.h>

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

uint16_t enabled = 0;
int active_channel = -1;

void process_minicom_command(char *command) {
  int channel = -1;

  if (strncmp(command, "enable", 6) == 0) {
    if (command[7] >= '0' && command[7] <= '9') {
      channel = command[7] - '0' - 1;
      servo_enable(channel, 1);
      enabled = 1;
      active_channel = channel;
    } else {
      printk("Invalid command\n");
    }
  } else if (strncmp(command, "disable", 7) == 0) {
    if (command[8] >= '0' && command[8] <= '9') {
      channel = command[8] - '0' - 1;
      servo_enable(channel, 0);
      enabled = 0;
      active_channel = -1;
    } else {
      printk("Invalid command\n");
    }
  } else {
    enabled = 0;
    active_channel = -1;
    printk("Invalid command\n");
  }
}

void process_keypad_input(uint8_t *row, uint8_t *col) {
  static char angle_str[4] = {0};  // buffer for angle input
  static int angle_idx = 0;  // index of the string

  if ((active_channel >= 0)) {
    char key = keypad_read();
    if (key != '\0') {
      if (key == '#') {
        if (angle_idx > 0) {
          printk("*User enters %s# to keypad*\n", angle_str);
          angle_str[angle_idx] = '\0';
          int angle = atoi(angle_str);
          // Validate the angle (must be between 0 and 180 degrees)
          if (angle <= 180) {
            servo_set(active_channel, angle); // Set the servo angle
            printk("Setting channel %d to angle %d\n", active_channel + 1, angle);
          } else {
            printk("Invalid angle.\n");
          }
          lcd_clear();
          *row = 0; // Reset cursor position for LCD
          *col = 0;
          angle_idx = 0; // Reset for next input
          
          // Manually clear angle_str array
          for (size_t i = 0; i < sizeof(angle_str); i++) {
            angle_str[i] = '\0';
          }
        }
      } else if ((key >= '0') && (key <= '9') && (angle_idx < 3)) {
        angle_str[angle_idx++] = key;
        key_display(key, row, col);
      }

    }
  }
}

/*
 * main():
*/
int main() {
  // initialize the uart and keypad
  systick_init();
  uart_init(115200);
  keypad_init();

  // // set GPIO
  // onboard LED (D13)
  gpio_init(GPIO_A, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  // SERVO 1 (A0)
  gpio_init(GPIO_A, 0, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  // SERVO 2 (A1)
  gpio_init(GPIO_A, 1, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);

  // // initialize the i2c_master and lcd_driver
  timer_init(3, 16000, 3000); // allow the onboard led to blink every 3 seconds
  i2c_master_init(80);
  lcd_driver_init();
  lcd_clear();

  uint8_t row = 0; //lcd cursor
  uint8_t col = 0; //lcd cursor
  const uint32_t uart_interval = 10000;   // 10 seconds
  uint32_t last_uart_time = systick_get_ticks() - uart_interval;
  

  printk("\nWelecome to Servo Controller!\nCommands\n  enable <ch>:  Enable servo channel\n");
  printk("  disable <ch>: Disable servo channel\n  Set the servo angle using the keypad\n\n");


  char buffer[128];
  while (1) {
    uint32_t current_time = systick_get_ticks();
    if ((current_time - last_uart_time) >= uart_interval) {
      printk("> ");
      if (uart_read(STDIN_FILENO, buffer, sizeof(buffer) - 1) > 0) {
        process_minicom_command(buffer);
      }
      last_uart_time = current_time;
    }
    if (enabled) {
      process_keypad_input(&row, &col);
    }
  }
  return 0;
}
