
# STM32F411 Timer Assignment

Tugas Mandiri Mikroprosesor dan Antarmuka

## Perangkat dan Software
- STM32F411CEU6 Black Pill
- STM32CubeIDE
- WeAct HID Flash
- Kabel USB-C

## Studi Kasus
1. Blinking Multiple LEDs at Different Frequencies
   Menggunakan TIM2, TIM3, dan TIM4 dengan
   frekuensi 1 Hz, 2 Hz, dan 4 Hz.

2. Debouncing a Pushbutton with a Timer
   Menggunakan EXTI0, TIM5, dan semaphore flag
   dengan interval debounce 20 ms.

3. Push to Blink
   LED berkedip selama tombol KEY ditekan
   dan berhenti ketika tombol dilepas.

## Konfigurasi
- Pemrograman bare-metal tanpa HAL.
- Clock HSI 16 MHz.
- LED bawaan PC13.
- Pushbutton PA0.
- Flash address 0x08004000.
- Upload melalui HID Bootloader.

## Cara Build
Salin kode studi kasus yang ingin dijalankan
ke main.c pada proyek STM32CubeIDE.
Gunakan linker script yang disertakan.
Build secara terpisah untuk setiap studi kasus.

## Keterbatasan Pengujian
Pengujian LED menggunakan LED bawaan PC13.
Output PA5 dan PA6 telah dikonfigurasi dalam
program, tetapi belum diverifikasi secara fisik
karena keterbatasan komponen eksternal.
