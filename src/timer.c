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
#include <timer.h>
#include <rcc.h>
#include <nvic.h>
#include <gpio.h>

#define UNUSED __attribute__((unused))
#define TIM2_IRQ_NUMBER (28)
#define TIM3_IRQ_NUMBER (29)
#define TIM4_IRQ_NUMBER (30)
#define TIM5_IRQ_NUMBER (50)


struct tim2_5* const timer_base[] = {(void *)0x0,    // N/A - Don't fill out
                                     (void *)0x0,    // N/A - Don't fill out
                                     (void *)0x40000000, // TIMER 2 Base Address
                                     (void *)0x40000400, // TIMER 3 Base Address
                                     (void *)0x40000800, // TIMER 4 Base Address
                                     (void *)0x40000C00};  // TIMER 5 Base Address



/*
* Starts the timer
*
* @param timer      - The timer
* @param prescaler  - Prescalar for clock
* @param Period     - Period of the timer interrupt
*/
void timer_init(UNUSED int timer, UNUSED uint32_t prescalar, UNUSED uint32_t period) {
  if (timer < 2 || timer > 5) return; // Check for valid timer
  struct tim2_5* tim = timer_base[timer];
  // TODO:1. Enable the timer clock in RCC 
  struct rcc_reg_map *rcc = RCC_BASE;
  switch (timer)
  {
  case 2:
    rcc->apb1_enr |= TIM2_CLKEN;
    nvic_irq(TIM2_IRQ_NUMBER, IRQ_ENABLE);
    break;
  case 3:
    rcc->apb1_enr |= TIM3_CLKEN;
    nvic_irq(TIM3_IRQ_NUMBER, IRQ_ENABLE);
    break;
  case 4:
    rcc->apb1_enr |= TIM4_CLKEN;
    nvic_irq(TIM4_IRQ_NUMBER, IRQ_ENABLE);
    break;
  case 5:
    rcc->apb1_enr |= TIM5_CLKEN;
    nvic_irq(TIM5_IRQ_NUMBER, IRQ_ENABLE);
    break;
  default:
    break;
  }
  // 2. Set the prescalar value 
  // fCK_PSC / (PSC[15:0] + 1).
  tim->psc = prescalar - 1;
  // 3. Set the auto-reload value
  tim->arr = period - 1;
  // 4. Enable the timer and its interrupt
  tim->dier |= 1; // Update interrupt enable
  tim->cr1 |= 1; // Enable the timer
}

/*
* Stops the timer
*
* @param timer      - The timer
*/
void timer_disable(UNUSED int timer) {
  if (timer < 2 || timer > 5) return; // Check for valid timer
  struct tim2_5* tim = timer_base[timer];
  // Disable the timer
  tim->cr1 &= ~1;
  struct rcc_reg_map *rcc = RCC_BASE;
  switch (timer)
  {
  case 2:
    rcc->apb1_enr &= ~TIM2_CLKEN;
    break;
  case 3:
    rcc->apb1_enr &= ~TIM3_CLKEN;
    break;
  case 4:
    rcc->apb1_enr &= ~TIM4_CLKEN;
    break;
  case 5:
    rcc->apb1_enr &= ~TIM5_CLKEN;
    break;
  default:
    break;
  }
}


/*
  * Clears the timer interrupt bit
*
  * @param timer      - The timer
  */
void timer_clear_interrupt_bit(UNUSED int timer) {
  if (timer < 2 || timer > 5) return; // Check for valid timer
  struct tim2_5* tim = timer_base[timer];
  // Clear the update interrupt flag
  tim->sr &= ~1;
}

volatile uint8_t ledstate = 0;
void tim3_irq_handler() {
  struct tim2_5* tim3 = timer_base[3];
  if (tim3->sr & TIM_SR_UIF) {
    if (ledstate) {
      gpio_clr(GPIO_A, 5);
    } else {
      gpio_set(GPIO_A, 5);
    }
    
    ledstate = !ledstate;
    timer_clear_interrupt_bit(3);
  }
}
