
#include <stdint.h>

// 2 = Debouncing Pushbutton
// 3 = Push to Blink
#define PROGRAM 2

#define REG(a) (*(volatile uint32_t *)(a))

#define RCC_CR       REG(0x40023800)
#define RCC_CFGR     REG(0x40023808)
#define RCC_AHB1ENR  REG(0x40023830)
#define RCC_APB1ENR  REG(0x40023840)
#define RCC_APB2ENR  REG(0x40023844)

#define GPIOA_MODER  REG(0x40020000)
#define GPIOA_PUPDR  REG(0x4002000C)
#define GPIOA_IDR    REG(0x40020010)

#define GPIOC_MODER  REG(0x40020800)
#define GPIOC_ODR    REG(0x40020814)
#define GPIOC_BSRR   REG(0x40020818)

#define SYSCFG_EXTICR1 REG(0x40013808)
#define EXTI_IMR      REG(0x40013C00)
#define EXTI_RTSR     REG(0x40013C08)
#define EXTI_FTSR     REG(0x40013C0C)
#define EXTI_PR       REG(0x40013C14)

#define TIM2 0x40000000UL
#define TIM5 0x40000C00UL

#define TIM_CR1(t)  REG((t) + 0x00)
#define TIM_DIER(t) REG((t) + 0x0C)
#define TIM_SR(t)   REG((t) + 0x10)
#define TIM_EGR(t)  REG((t) + 0x14)
#define TIM_CNT(t)  REG((t) + 0x24)
#define TIM_PSC(t)  REG((t) + 0x28)
#define TIM_ARR(t)  REG((t) + 0x2C)

#define NVIC_ISER0 REG(0xE000E100)
#define NVIC_ISER1 REG(0xE000E104)
#define SCB_VTOR   REG(0xE000ED08)

// Semaphore / flag
volatile uint32_t button_flag = 0;
volatile uint32_t button_sample = 0;
volatile uint32_t blink_flag = 0;

// Interrupt eksternal tombol PA0
void EXTI0_IRQHandler(void)
{
    // Mask EXTI selama debounce
    EXTI_IMR &= ~1U;
    EXTI_PR = 1U;

    // Mulai timer one-pulse 20 ms
    TIM_CNT(TIM5) = 0;
    TIM_SR(TIM5) = 0;
    TIM_CR1(TIM5) |= 1U;
}

// Interrupt debounce 20 ms
void TIM5_IRQHandler(void)
{
    TIM_SR(TIM5) = 0;
    EXTI_PR = 1U;

    // Sampling tombol setelah debounce
    button_sample = !(GPIOA_IDR & 1U);

    // Kirim semaphore ke main
    button_flag = 1;

    // Aktifkan kembali EXTI
    EXTI_IMR |= 1U;
}

// Interrupt timer kedip
void TIM2_IRQHandler(void)
{
    TIM_SR(TIM2) = 0;
    blink_flag = 1;
}

int main(void)
{
    uint32_t pressed = 0;

    // Vector table HID Bootloader
    SCB_VTOR = 0x08004000;

    // HSI 16 MHz
    RCC_CR |= 1U;
    while (!(RCC_CR & (1U << 1)));

    RCC_CFGR &= ~3U;
    while (RCC_CFGR & (3U << 2));

    RCC_CFGR &= ~((15U << 4) |
                  (7U << 10) |
                  (7U << 13));

    // Clock GPIOA dan GPIOC
    RCC_AHB1ENR |= (1U << 0) | (1U << 2);

    // Clock TIM2 dan TIM5
    RCC_APB1ENR |= (1U << 0) | (1U << 3);

    // Clock SYSCFG
    RCC_APB2ENR |= (1U << 14);

    // LED PC13 output, kondisi OFF
    GPIOC_BSRR = (1U << 13);
    GPIOC_MODER &= ~(3U << 26);
    GPIOC_MODER |= (1U << 26);

    // PA0 input dengan pull-up
    GPIOA_MODER &= ~3U;
    GPIOA_PUPDR &= ~3U;
    GPIOA_PUPDR |= 1U;

    // EXTI0 terhubung ke PA0
    SYSCFG_EXTICR1 &= ~15U;

    // Deteksi tekan dan lepas
    EXTI_RTSR |= 1U;
    EXTI_FTSR |= 1U;
    EXTI_PR = 1U;

    // TIM5: debounce 20 ms
    TIM_CR1(TIM5) = (1U << 3);
    TIM_PSC(TIM5) = 15999;
    TIM_ARR(TIM5) = 19;
    TIM_EGR(TIM5) = 1;
    TIM_SR(TIM5) = 0;
    TIM_DIER(TIM5) = 1;

#if PROGRAM == 3
    // TIM2: toggle setiap 250 ms
    TIM_CR1(TIM2) = 0;
    TIM_PSC(TIM2) = 15999;
    TIM_ARR(TIM2) = 249;
    TIM_EGR(TIM2) = 1;
    TIM_SR(TIM2) = 0;
    TIM_DIER(TIM2) = 1;
    NVIC_ISER0 = (1U << 28);
    TIM_CR1(TIM2) = 1;
#endif

    // NVIC EXTI0 IRQ6
    NVIC_ISER0 = (1U << 6);

    // NVIC TIM5 IRQ50
    NVIC_ISER1 = (1U << 18);

    EXTI_IMR |= 1U;

    while (1)
    {
        // Ambil flag secara atomik
        __asm volatile ("cpsid i" ::: "memory");

        uint32_t event = button_flag;
        uint32_t sample = button_sample;

        button_flag = 0;

#if PROGRAM == 3
        uint32_t tick = blink_flag;
#endif
        blink_flag = 0;

        __asm volatile ("cpsie i" ::: "memory");

        if (event)
        {
            if (sample != pressed)
            {
                pressed = sample;

#if PROGRAM == 2
                // Toggle sekali setiap ditekan
                if (pressed)
                    GPIOC_ODR ^= (1U << 13);
#endif

#if PROGRAM == 3
                // LED mati ketika dilepas
                if (!pressed)
                    GPIOC_BSRR = (1U << 13);
#endif
            }
        }

#if PROGRAM == 3
        // Berkedip hanya selama ditekan
        if (tick && pressed)
            GPIOC_ODR ^= (1U << 13);
#endif
    }
}
