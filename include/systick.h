/**
* @file
*
* @brief
*
* @date
*
* @author
*/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

void systick_init();

void systick_delay();

uint32_t systick_get_ticks();

void systick_c_handler();

#endif /* _SYSTICK_H_ */
