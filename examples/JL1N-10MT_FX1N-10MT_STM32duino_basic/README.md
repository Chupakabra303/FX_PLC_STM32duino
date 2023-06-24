# Базовый пример на STM32duino для ПЛК JL1N-10MT (FX1N-10MT)

MCU: STM32F103RCT6

+ DI/DO/AI/AO
+ Power monitor
+ RTC (без батареи)
+ Автоматический переход в режим загрузчика с помощью кода инициализации бутлодера по UART.
AN3155, USART bootloader code sequence. Функция перехода в загрузчик bootloaderInit()
+ Лог в UART (Serial1)
