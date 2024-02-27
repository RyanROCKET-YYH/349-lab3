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

#define UNUSED __attribute__((unused))

/** @brief The systick timer register map. */
struct stk_reg_map {
    volatile uint32_t CTRL;      /**<  control and status register (STK_CTRL) */
    volatile uint32_t LOAD;      /**<  SysTick reload value register (STK_LOAD) */
    volatile uint32_t VAL;     /**<  SysTick current value register (STK_VAL) */
    volatile uint32_t CALIB;     /**<  SysTick calibration value register (STK_CALIB) */
    
};

void systick_init() {

}

/**
* systick_delay():
* @brief delays the processor for ticks milliseconds
*
*/
void systick_delay(UNUSED uint32_t ticks) {

}

uint32_t systick_get_ticks() {
    return -1;
}

void systick_c_handler() {

}
