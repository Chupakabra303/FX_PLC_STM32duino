/*
 * STM32duinoBootloaderInit.h
 * Created on: 03.02.2023
 * Author: Chupakabra303
 */
 
#ifndef STM32DUINOBOOTLOADERINIT_H_
#define STM32DUINOBOOTLOADERINIT_H_

/* Сюда добавляются #define BOOTLOADER_ADDR для System memory bootloader firmware address из документа AN2606.
Можно использовать внешний #define BOOTLOADER_ADDR */
#if !defined(BOOTLOADER_ADDR)
  #if defined(ARDUINO_GENERIC_F103VCTX) || defined(ARDUINO_GENERIC_F103RCTX)
    #define BOOTLOADER_ADDR 0x1FFFF000
  #elif defined(ARDUINO_GENERIC_F030C8TX)
    #define BOOTLOADER_ADDR 0x1FFFEC00
  #else
    #error "No #define BOOTLOADER_ADDR for this MCU. See AN2606 (STM32... devices bootloader)"
  #endif
#endif

/* Функция выполняет переход во внутренний загрузчик STM32 */
void bootloaderInit(void) {
  // volatile uint32_t bootloaderAddr = 0x1FFFF000;
  // Step: Set jump memory location for system memory. Use address with 4 bytes offset which specifies jump location where program starts
  void (*SysMemBootJump) (void) = (void (*)(void)) *(uint32_t *)(BOOTLOADER_ADDR + 4); // System Memory reset vector is one word after addr
  // Step: Disable RCC, set it to default (after reset) settings. Internal clock, no PLL, etc.
#if defined(USE_HAL_DRIVER)
  HAL_RCC_DeInit();
#endif // defined(USE_HAL_DRIVER)
#if defined(USE_STDPERIPH_DRIVER)
  RCC_DeInit();
#endif // defined(USE_STDPERIPH_DRIVER)
  // Step: Disable systick timer and reset it to default values
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  // Step: Disable all interrupts
  __disable_irq();
  // Step: Set main stack pointer. This step must be done last otherwise local variables in this function don't have proper value since stack pointer is located on different position
  // Set direct address location which specifies stack pointer in SRAM location
  __set_MSP(*(uint32_t *)BOOTLOADER_ADDR);
  // jump
  SysMemBootJump();
  // while (1);
}

#endif