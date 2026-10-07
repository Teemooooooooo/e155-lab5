// button_interrupt.c
// Ellen Yu ellyu@g.hmc.edu Oct. 5 2026

#include "main.h"

int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}

volatile int count = 0;
volatile float speed = 0;
volatile bool direction = 0;

int main(void) {
    
    // Enable PA8 as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(BUTTON_PIN)); // Set PA7 as pull-up (PUPD7 = 01)

    // Enable PA10 as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON2_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(BUTTON2_PIN)); // Set PA10 as pull-up (PUPD10 = 01)

    // Initialize timer
    RCC->APB1ENR1 |= (1 << 0); // TIM2EN
    initTIM(DELAY_TIM);


    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= (1 << 0); // SYSCFGEN
    // 2. Configure EXTICR for the input button interrupt
    // EXTI7 is bits 14:12 of EXTICR2 (EXTICR[1] in C). Port A is 0b000, so clearing the field selects PA7.
    SYSCFG->EXTICR[1] &= ~(0b111 << 12);
    // Configure EXTICR for EXTI10 
    SYSCFG->EXTICR[1] &= ~(0b111 << 12);
    // Enable interrupts globally
    __enable_irq();

    // setting priority
    __NVIC_SetPriority(TIM2_IRQn, 1);
    __NVIC_SetPriority(EXTI9_5_IRQn, 2);
    __NVIC_SetPriority(EXTI15_10_IRQn, 3);

    // Configure interrupt for rising and failing edge of GPIO pin for line 7
    // 1. Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN));
    // 2. Enable rising edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << 23);

    // Configure interrupt for rising and failing edge of GPIO pin for line 10
    // 1. Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON2_PIN));
    // 2. Enable rising edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(BUTTON2_PIN));
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON2_PIN));
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[1] |= (1 << 8);

    TIMx->ARR = 9999;// Set timer max count
    TIMx->EGR |= 1;     // Force update
    TIMx->SR &= ~(0x1); // Clear UIF
    

    while(1){
        // if the timer is at 1 second   
        if ((TIM2->SR) &= 1){
            TIMx->SR &= ~(0x1); // Clear UIF
            // calculate speed
            speed = count / (408f *4);

            if (direction){
                printf("CCW \n");
            }else{
                printf("CW \n");
            }
            count = 0;
            printf("speed is %f rps \n", speed)
        }
    }

}



void EXTI9_5_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(BUTTON_PIN));
        count += 1;

        printf("Hello World %d!\n");
        // A = B, positive direction
        // A != B, negative direction 
        if (digitalRead(gpioPinOffset(BUTTON_PIN)) == digitalRead(gpioPinOffset(BUTTON2_PIN))){
            direction = 1;
        }else{
            direction = 0;
        }
    }
}

void EXTI15_10_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON2_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(BUTTON2_PIN));
        
        count += 1;
        // A = B, negative direction
        // A != B, positive direction 
        if (digitalRead(gpioPinOffset(BUTTON_PIN)) == digitalRead(gpioPinOffset(BUTTON2_PIN))){
            direction = 0;
        }else{
            direction = 1;
        }

    }
}

