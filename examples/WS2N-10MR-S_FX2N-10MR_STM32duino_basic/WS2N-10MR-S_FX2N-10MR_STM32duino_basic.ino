// MCU APM32F030C8T6 (STM32F030C8T6)
#include "..\STM32duinoBootloaderInit.h"
#include <STM32RTC.h> // https://github.com/stm32duino/STM32RTC

// Serial1 в этом проекте используется для отладки. См. setup();
// HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);

/* ------------------- Дискретные входы Xn ------------------- */
const int X0_PIN = PA0;
const int X1_PIN = PB3;
const int X2_PIN = PB4;
const int X3_PIN = PA1;
const int X4_PIN = PF1;
const int X5_PIN = PC14;
const int POWER_MONITORING_PIN = PB1; // мониторинг питания (делитель напряжения на входе +24В)

/* ------------------- Дискретные выходы Yn ------------------- */
const int RUN_LED_PIN = PB10;
const int ERROR_LED_PIN = PB11;
const int Y0_PIN = PA4;
const int Y1_PIN = PA7;
const int Y2_PIN = PB14;
const int Y3_PIN = PB15;

/* ------------------- Аналоговые входы-выходы ------------------- */
const int AD0_PIN = PA5;
const int AD1_PIN = PB0;
const int PWN_PIN = PA6;
/* Машинный перевод: [Выход ШИМ] Один встроенный выход широтно-импульсной модуляции ШИМ, потому что оптическая связь имеет задержку,
 этот не проходит через оптическую связь, частота 1 кГц, рабочий цикл соответствует диапазону рабочего цикла D8033 0-1000 ,
 и может использоваться как аналог после фильтрации RC Использование количества */

/* ------------------- RTC ------------------- */
STM32RTC& rtc = STM32RTC::getInstance();

bool poweroff;
int counter;

void setup() {
  Serial.setRx(PA10); // Отладка на USART1 вместо HardwareSerial Serial1(PA10, PA9);
  Serial.setTx(PA9);
  //while(!Serial);
  Serial.begin(115200);
  Serial.println("\nInitialization...");

  /* ------------------- Дискретные входы ------------------- */  
  pinMode(X0_PIN, INPUT); // Внешняя подтяжка к +3.3В, инвертированная логика
  pinMode(X1_PIN, INPUT);
  pinMode(X2_PIN, INPUT);
  pinMode(X3_PIN, INPUT);
  pinMode(X4_PIN, INPUT);
  pinMode(X5_PIN, INPUT);
  pinMode(POWER_MONITORING_PIN, INPUT); // Подключен к +24В (на конденсаторе)
  
  /* ------------------- Дискретные выходы ------------------- */
  pinMode(RUN_LED_PIN, INPUT); // Сначала инициализируем как вход в высокоимпедансном состоянии
  digitalWrite(RUN_LED_PIN, HIGH); // Инвертированная логика LED, инициализация с HIGH
  pinMode(RUN_LED_PIN, OUTPUT); // Потом переключаем в Output
  pinMode(ERROR_LED_PIN, INPUT); // Сначала инициализируем как вход в высокоимпедансном состоянии
  digitalWrite(ERROR_LED_PIN, HIGH); // Инвертированная логика LED, инициализация с HIGH
  pinMode(ERROR_LED_PIN, OUTPUT); // Потом переключаем в Output
  pinMode(Y0_PIN, OUTPUT);
  pinMode(Y1_PIN, OUTPUT);
  pinMode(Y2_PIN, OUTPUT);
  pinMode(Y3_PIN, OUTPUT);
  
  /* ------------------- Аналоговые входы-выходы ------------------- */
  pinMode(PWN_PIN, OUTPUT); // Потом переключаем в Output
  analogReadResolution(12);
  //analogWriteResolution(12);
  analogWriteFrequency(50);

  /* ------------------- RTC ------------------- */
  //rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format
}

void loop() {
  while (Serial.available()) {
    uint8_t ch = (uint8_t)Serial.read();
    if (ch == 'x') { // Переход в режим загрузчика при приеме по последовательному порту специального символа 'x'
      Serial.println("--------------- bootloaderInit() ---------------");
      Serial.flush();
      bootloaderInit();
    } else if (ch == 0x7F) {
      /* Автоматический переход в режим загрузчика с помощью кода инициализации бутлодера по UART. AN3155, USART bootloader code sequence. Позволяет автоматически, без использования пина BOOT0 загружать скетч из Arduino IDE по нажатию кнопки Загрузка! */
      bootloaderInit(); // 
    }
  }  
  if (!digitalRead(X5_PIN)) { // Используем вход X5 как программный reset или переход в режим загрузчика для прошивки по UART
    //NVIC_SystemReset();
    Serial.println("--------------- bootloaderInit() ---------------");
    Serial.flush();       
    bootloaderInit(); // STM32duinoBootloaderInit.h
  }

  /* ------- PWN_PIN как аналоговый выход ------- */
  counter = (counter >= 255)? 0: counter+1;
  digitalToggle(RUN_LED_PIN);
  //digitalToggle(PWN_PIN);
  analogWrite(PWN_PIN, counter);

  /* ------- Мониторинг питания ------- */
  if (!digitalRead(POWER_MONITORING_PIN) && !poweroff) {
    Serial.println("--------------- Power OFF ---------------");
    poweroff = true;
  }
  if (digitalRead(POWER_MONITORING_PIN) && poweroff) {
    Serial.println("--------------- Power ON ---------------");
    poweroff = false;    
  }

  /* ------- Чтение RTC ------- */
  Serial.printf("%02d/%02d/%02d ", rtc.getDay(), rtc.getMonth(), rtc.getYear());
  Serial.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
    
  digitalWrite(Y0_PIN, !digitalRead(X0_PIN));
  digitalWrite(Y1_PIN, !digitalRead(X1_PIN));
  //digitalWrite(Y2_PIN, !digitalRead(X2_PIN));
  //digitalWrite(Y3_PIN, !digitalRead(X3_PIN));  
  digitalWrite(Y2_PIN, rtc.getSeconds() == 30);
  digitalWrite(Y3_PIN, rtc.getSeconds() == 33);

  digitalWrite(ERROR_LED_PIN, digitalRead(POWER_MONITORING_PIN)); // Включаем Error LED при пропаже питания. Инвертированная логика LED
  
  Serial.printf("X0-X5: %d %d %d %d %d %d\n", !digitalRead(X0_PIN), !digitalRead(X1_PIN), !digitalRead(X2_PIN), !digitalRead(X3_PIN), !digitalRead(X4_PIN), !digitalRead(X5_PIN));
  Serial.printf("POWER_MONITORING: %d\n", digitalRead(POWER_MONITORING_PIN));
  Serial.printf("AD0, AD1: %d %d\n", analogRead(AD0_PIN), analogRead(AD1_PIN));

  delay(100);
}
