
#include <stdint.h>

#define REG(addr) (*(volatile uint32_t *)(addr))

#define RCC_CR       REG(0x40023800)
#define RCC_CFGR     REG(0x40023808)
#define RCC_AHB1ENR  REG(0x40023830)
#define RCC_APB1ENR  REG(0x40023840)

#define GPIOA_MODER  REG(0x40020000)
#define GPIOA_ODR    REG(0x40020014)
#define GPIOC_MODER  REG(0x40020800)
#define GPIOC_ODR    REG(0x40020814)

#define TIM2 0x40000000UL
#define TIM3 0x40000400UL
#define TIM4 0x40000800UL

#define TIM_CR1(t)   REG((t) + 0x00)
#define TIM_DIER(t)  REG((t) + 0x0C)
#define TIM_SR(t)    REG((t) + 0x10)
#define TIM_EGR(t)   REG((t) + 0x14)
#define TIM_PSC(t)   REG((t) + 0x28)
#define TIM_ARR(t)   REG((t) + 0x2C)

#define NVIC_ISER0   REG(0xE000E100)
#define SCB_VTOR     REG(0xE000ED08)

void Timer_Init(uint32_t timer, uint32_t arr)
{
    TIM_CR1(timer) = 0;
    TIM_PSC(timer) = 15999;
    TIM_ARR(timer) = arr;
    TIM_EGR(timer) = 1;
    TIM_SR(timer) = 0;
    TIM_DIER(timer) = 1;
    TIM_CR1(timer) = 1;
}

void TIM2_IRQHandler(void)
{
    TIM_SR(TIM2) = 0;
    GPIOC_ODR ^= (1U << 13);
}

void TIM3_IRQHandler(void)
{
    TIM_SR(TIM3) = 0;
    GPIOA_ODR ^= (1U << 5);
}

void TIM4_IRQHandler(void)
{
    TIM_SR(TIM4) = 0;
    GPIOA_ODR ^= (1U << 6);
}

int main(void)
{
    // Vector table untuk HID Bootloader
    SCB_VTOR = 0x08004000;

    // Gunakan HSI 16 MHz
    RCC_CR |= 1U;
    while (!(RCC_CR & (1U << 1)));

    RCC_CFGR &= ~3U;
    while (RCC_CFGR & (3U << 2));

    // Prescaler AHB dan APB = 1
    RCC_CFGR &= ~((15U << 4) |
                  (7U << 10) |
                  (7U << 13));

    // Clock GPIOA dan GPIOC
    RCC_AHB1ENR |= (1U << 0) | (1U << 2);

    // Clock TIM2, TIM3, TIM4
    RCC_APB1ENR |= 7U;

    // PC13 sebagai output
    GPIOC_ODR |= (1U << 13);
    GPIOC_MODER &= ~(3U << 26);
    GPIOC_MODER |= (1U << 26);

    // PA5 dan PA6 sebagai output
    GPIOA_ODR &= ~((1U << 5) | (1U << 6));
    GPIOA_MODER &= ~((3U << 10) | (3U << 12));
    GPIOA_MODER |= (1U << 10) | (1U << 12);

    // Konfigurasi timer
    Timer_Init(TIM2, 499);
    Timer_Init(TIM3, 249);
    Timer_Init(TIM4, 124);

    // Aktifkan interrupt pada NVIC
    NVIC_ISER0 = (1U << 28) |
                 (1U << 29) |
                 (1U << 30);

    while (1)
    {
        __asm volatile ("wfi");
    }
}
