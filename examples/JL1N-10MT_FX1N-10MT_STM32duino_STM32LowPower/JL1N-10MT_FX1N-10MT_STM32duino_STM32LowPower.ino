// MCU STM32F103RCT6
#include "..\STM32duinoBootloaderInit.h"
#include <STM32LowPower.h>

// Serial1 в этом проекте используется для отладки. См. setup();
// HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);

/* ------------------- Дискретные входы Xn ------------------- */
const int X0_PIN = PA0;
const int X1_PIN = PA1;
const int X2_PIN = PC5;
const int X3_PIN = PC6;
const int X4_PIN = PC7;
const int X5_PIN = PC4;
const int POWER_MONITORING_PIN = PB7; // Выход VOut +5В с MP1584EN, мониторинг питания

/* ------------------- Дискретные выходы Yn ------------------- */
//const int ERROR_LED_PIN; Нет
const int RUN_LED_PIN = PB6;
const int Y0_PIN = PA2; // PA2 и он же на PD2
const int Y1_PIN = PC8;
const int Y2_PIN = PB10;
const int Y3_PIN = PB11;

/* ------------------- Аналоговые входы-выходы ------------------- */
const int AIN_PIN = PA4; // Бывают разные модификации. У меня AIN + AOUT
const int AOUT_PIN = PA5;

void setup() {
  Serial.setRx(PA10); // Отладка на USART1 вместо HardwareSerial Serial1(PA10, PA9);
  Serial.setTx(PA9);
  //while(!Serial);
  Serial.begin(115200);
  Serial.println("\nInitialization...");

  /* ------------------- Дискретные входы ------------------- */  
  pinMode(X0_PIN, INPUT_PULLUP);
  pinMode(X1_PIN, INPUT_PULLUP);
  pinMode(X2_PIN, INPUT_PULLUP);
  pinMode(X3_PIN, INPUT_PULLUP);
  pinMode(X4_PIN, INPUT_PULLUP);
  pinMode(X5_PIN, INPUT_PULLUP);
  pinMode(POWER_MONITORING_PIN, INPUT); // Выход VOut +5В с MP1584EN, мониторинг питания
  //pinMode(POWER_MONITORING_PIN, INPUT_PULLDOWN);
  
  /* ------------------- Дискретные выходы ------------------- */
  pinMode(RUN_LED_PIN, INPUT);
  digitalWrite(RUN_LED_PIN, HIGH); //pinMode(RUN_LED_PIN, INPUT_PULLUP);
  pinMode(RUN_LED_PIN, OUTPUT_OPEN_DRAIN); // RUN LED, запитан от +5В, поэтому OUTPUT_OPEN_DRAIN
  pinMode(Y0_PIN, INPUT);
  digitalWrite(Y0_PIN, HIGH); //pinMode(Y0_PIN, INPUT_PULLUP); // PA2 и он же на PD2
  pinMode(Y0_PIN, OUTPUT); // Y0
  pinMode(Y1_PIN, INPUT);
  digitalWrite(Y1_PIN, HIGH); //pinMode(Y1_PIN, INPUT_PULLUP);
  pinMode(Y1_PIN, OUTPUT); // Y1
  pinMode(Y2_PIN, INPUT);
  digitalWrite(Y2_PIN, HIGH); //pinMode(Y2_PIN, INPUT_PULLUP);
  pinMode(Y2_PIN, OUTPUT); // Y2
  pinMode(Y3_PIN, INPUT);
  digitalWrite(Y3_PIN, HIGH); //pinMode(Y3_PIN, INPUT_PULLUP);
  pinMode(Y3_PIN, OUTPUT); // Y3
  
  LowPower.begin();
  // LowPower.attachInterruptWakeup(X0_PIN, repetitionsIncrease, CHANGE, DEEP_SLEEP_MODE);
  LowPower.attachInterruptWakeup(X0_PIN, NULL, CHANGE, DEEP_SLEEP_MODE);  
  LowPower.attachInterruptWakeup(X1_PIN, NULL, CHANGE, DEEP_SLEEP_MODE);
  LowPower.attachInterruptWakeup(X2_PIN, NULL, CHANGE, DEEP_SLEEP_MODE);
}


void loop() {
  /*while (Serial.available()) {
    uint8_t ch = (uint8_t)Serial.read();
    if (ch == 'x') { // Переход в режим загрузчика при приеме по последовательному порту специального символа 'x'
      Serial.println("--------------- bootloaderInit() ---------------");
      Serial.flush();
      bootloaderInit();
    } else if (ch == 0x7F) {
      // Автоматический переход в режим загрузчика с помощью кода инициализации бутлодера по UART. AN3155, USART bootloader code sequence. Позволяет автоматически, без использования пина BOOT0 загружать скетч из Arduino IDE по нажатию кнопки Загрузка!
      bootloaderInit(); // 
    }
  }  
  if (!digitalRead(X5_PIN)) { // Используем вход X5 как программный reset или переход в режим загрузчика для прошивки по UART
    //NVIC_SystemReset();
    Serial.println("--------------- bootloaderInit() ---------------");
    Serial.flush();
    bootloaderInit(); // STM32duinoBootloaderInit.h
  }*/


  //digitalWrite(Y0_PIN, digitalRead(X0_PIN));
  //digitalWrite(Y1_PIN, digitalRead(X1_PIN));
  //digitalWrite(Y2_PIN, digitalRead(X2_PIN));
  //digitalWrite(Y3_PIN, digitalRead(X3_PIN));  

  Serial.printf("X0-X5: %d %d %d %d %d %d\n", !digitalRead(X0_PIN), !digitalRead(X1_PIN), !digitalRead(X2_PIN), !digitalRead(X3_PIN), !digitalRead(X4_PIN), !digitalRead(X5_PIN));
  Serial.flush();
  digitalWrite(RUN_LED_PIN, !digitalRead(RUN_LED_PIN));
  delay(100);
  LowPower.deepSleep();
}

void repetitionsIncrease() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
  // repetitions ++;
}