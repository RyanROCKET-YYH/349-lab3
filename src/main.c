/**
 * @file main.c
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <gpio.h>
#include <i2c.h>
#include <printk.h>
#include <uart_polling.h>
#include <unistd.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
#include <systick.h>

int main( void ) {
    //systick_c_handler();
    uart_polling_init(115200);
    systick_init();
    // g_tick_count = 0;
    while(1){
        // uint32_t lastTick = g_tick_count;
        // uint32_t currentTick;

        // // Delay for 1000 milliseconds (1 second)
        // systick_delay(100);
        // printk("\nset delay 100 milliseconds%d\n");

        // // Get the total elapsed ticks (milliseconds) since the program started
        // currentTick = systick_get_ticks();
        // printk("\ncurrentTick= %d\n", currentTick);

        // // Calculate the elapsed time since the last print
        // uint32_t elapsedTime = currentTick - lastTick;
        // printk("\nelapsedTime= %d\n", elapsedTime);
        // lastTick = currentTick;
        systick_c_handler();
    }
   

    return 0;
}
