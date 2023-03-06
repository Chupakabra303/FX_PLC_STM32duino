// MCU APM32F030C8T6 (STM32F030C8T6)
#include "..\STM32duinoBootloaderInit.h"
#include <ModbusRTUServer.h> // https://www.arduino.cc/reference/en/libraries/arduinomodbus/

// Serial1 в этом проекте используется для Modbus RTU;
// HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);

/* ------------------- Modbus RTU на Serial1 ------------------- */
HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);
RS485Class _RS485(Serial1, PA9, -1, -1); // RS485Class(HardwareSerial& hwSerial, int txPin, int dePin, int rePin);

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
/* Перевод: [Выход ШИМ] Один встроенный выход широтно-импульсной модуляции ШИМ, потому что оптическая связь имеет задержку,
 этот не проходит через оптическую связь, частота 1 кГц, рабочий цикл соответствует диапазону рабочего цикла D8033 0-1000 ,
 и может использоваться как аналог после фильтрации RC Использование количества */

void setup() {
  ///Serial1.begin(115200);
  ///delay(100);
  ///Serial1.println("\nInitialization...");

  /* ------------------- Modbus RTU на Serial1 ------------------- */
  ModbusRTUServer.begin(_RS485, 1, 115200);
  ModbusRTUServer.configureHoldingRegisters(0x00, 10); // configure holding registers at address 0x00
  ModbusRTUServer.configureInputRegisters(0x00, 10); // configure input registers at address 0x00
  
  /* ------------------- Дискретные входы ------------------- */  
  pinMode(X0_PIN, INPUT); // Внешняя подтяжка к +3.3В, инвертированная логика
  pinMode(X1_PIN, INPUT);
  pinMode(X2_PIN, INPUT);
  pinMode(X3_PIN, INPUT);
  pinMode(X4_PIN, INPUT);
  pinMode(X5_PIN, INPUT);
  pinMode(POWER_MONITORING_PIN, INPUT); // Подключен к +24В (на конденсаторе)
  
  /* ------------------- Дискретные выходы ------------------- */
  pinMode(RUN_LED_PIN, INPUT); // Сначала инициализируем как вход в высокоимпенадсном состоянии
  digitalWrite(RUN_LED_PIN, HIGH); // Инвертированная логика LED, инициализация с HIGH
  pinMode(RUN_LED_PIN, OUTPUT); // Потом переключаем в Output

  pinMode(ERROR_LED_PIN, INPUT); // Сначала инициализируем как вход в высокоимпенадсном состоянии
  digitalWrite(ERROR_LED_PIN, HIGH); // Инвертированная логика LED, инициализация с HIGH
  pinMode(ERROR_LED_PIN, OUTPUT); // Потом переключаем в Output

  pinMode(Y0_PIN, OUTPUT);
  pinMode(Y1_PIN, OUTPUT);
  pinMode(Y2_PIN, OUTPUT);
  pinMode(Y3_PIN, OUTPUT);
  
  /* ------------------- Аналоговые входы-выходы ------------------- */
  analogReadResolution(12);
}

void loop() {
  if (!digitalRead(X5_PIN)) { // Используем вход X5 как программный reset или переход в режим загрузчика для прошивки по UART
    //NVIC_SystemReset();
    bootloaderInit(); // STM32duinoBootloaderInit.h
  }
  
  /* ------- Modbus RTU на Serial1 ------- */
  ModbusRTUServer.poll();
  ModbusRTUServer.inputRegisterWrite(0, digitalRead(POWER_MONITORING_PIN)); // Input Register 0 - состояние дискретых входов
  digitalWrite(Y0_PIN, ModbusRTUServer.holdingRegisterRead(0) & 1);
  digitalWrite(Y1_PIN, ModbusRTUServer.holdingRegisterRead(0) & 2);
  digitalWrite(Y2_PIN, ModbusRTUServer.holdingRegisterRead(0) & 4);
  digitalWrite(Y3_PIN, ModbusRTUServer.holdingRegisterRead(0) & 8);  
  
  digitalToggle(RUN_LED_PIN);
  
  ///digitalWrite(Y0_PIN, !digitalRead(X0_PIN));
  ///digitalWrite(Y1_PIN, !digitalRead(X1_PIN));
  ///digitalWrite(Y2_PIN, !digitalRead(X2_PIN));
  ///digitalWrite(Y3_PIN, !digitalRead(X3_PIN));  

  digitalWrite(ERROR_LED_PIN, digitalRead(POWER_MONITORING_PIN)); // Включаем Error LED при пропаже питания. Инвертированная логика LED
  
  ///Serial1.printf("X0-X5: %d %d %d %d %d %d\n", !digitalRead(X0_PIN), !digitalRead(X1_PIN), !digitalRead(X2_PIN), !digitalRead(X3_PIN), !digitalRead(X4_PIN), !digitalRead(X5_PIN));
  ///Serial1.printf("POWER_MONITORING: %d\n", digitalRead(POWER_MONITORING_PIN));
  ///Serial1.printf("AD0, AD1: %d %d\n", analogRead(AD0_PIN), analogRead(AD1_PIN));

  delay(100);
}
