Базовый пример на STM32duino для ПЛК WS2N-10MR-S (FX2N-10MR):

MCU APM32F030C8T6 (STM32F030C8T6)

+ DI/DO/AI/AO
+ Power monitor
+ RTC (без батареи)
+ Автоматический переход в режим загрузчика с помощью символа инициализации бутлодера по UART.
AN3155, USART bootloader code sequence. Функция перехода в загрузчик bootloaderInit()
+ Лог в UART (Serial1)
