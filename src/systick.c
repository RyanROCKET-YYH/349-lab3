/**
* @file systick.c
*
* @brief functions about systick interrupt
*
* @date 03/15/2024
*
* @author Yiying Li, Yuhong Yao
*/

#include <unistd.h>
#include <systick.h>
#include <printk.h>

/** @brief define UNUSE for unuse parameters */
#define UNUSED __attribute__((unused))

/** @brief The systick(STK) timer register map. */
struct stk_reg_map {
    volatile uint32_t CTRL;      /**<  control and status register (STK_CTRL) */
    volatile uint32_t LOAD;      /**<  SysTick reload value register (STK_LOAD) */
    volatile uint32_t VAL;       /**<  SysTick current value register (STK_VAL) */
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
/** @brief STK CTRL: COUNTFLAG bit */
#define STK_CTRL_COUNTFLAG (1 << 16)

/** @brief count the glocal tick */
volatile uint32_t g_tick_count = 0;

/**
* systick_init():
* @brief initialize the systick interrupt
*
*/
void systick_init() {
    
    struct stk_reg_map* stk = STK_BASE;

    // Frequency: 16MHz( 16000000 / 1000 -1 )
    stk->LOAD = (uint32_t)15999;
    // clear current value
    stk->VAL = (u_int16_t)0;

    // When ENABLE is set to 1, the counter loads the RELOAD value from the LOAD register and then counts down
    stk->CTRL |= STK_CTRL_EN;
    stk->CTRL |= STK_CTRL_TICKINT;
    stk->CTRL |= STK_CTRL_CLKSOURCE;
}

/**
* systick_delay():
* @brief delays the processor for ticks milliseconds
*
* @param ticks: to count the delay ticks
*/
void systick_delay(uint32_t ticks) {
    uint32_t start = g_tick_count;
    // check the delay time
    while (((g_tick_count - start) - ticks));
}

/**
* systick_get_ticks():
* @brief the total number of times the systick c handler was called
*
*/
uint32_t systick_get_ticks() {
    // the total number of times the systick c handler was called
    return g_tick_count;
}

/**
* systick_c_handler():
* @brief whenever systick interrupt happens(when 15999 counts down to 0), it will call the systick_c_handler via ivt automatically
* we have to make a glocal variable time(g_tick_count) as a clock count.
*
*/
void systick_c_handler() {
    // whenever call systick_c_handler, global time ++
    g_tick_count++;
}
