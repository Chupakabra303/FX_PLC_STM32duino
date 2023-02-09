// MCU STM32F103VCT6
#include "..\STM32duinoBootloaderInit.h"
#include <SlowSoftI2CMaster.h> // https://github.com/felias-fogg/SlowSoftI2CMaster
#include <ModbusRTUServer.h> // https://github.com/arduino-libraries/ArduinoModbus
#include <STM32RTC.h> // https://github.com/stm32duino/STM32RTC

// Serial1 в этом проекте используется для отладки. См. setup();
// HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);

/* ------------------- Modbus RTU на RS-485 (Serial4) ------------------- */
HardwareSerial Serial4(PC11, PC10);
RS485Class _RS485(Serial4, PC10, PA14, -1); // RS485Class(HardwareSerial& hwSerial, int txPin, int dePin, int rePin);

/* ------------------- Дискретные входы Xn ------------------- */
const int START_SWITCH_PIN = PB2; // PB2 - это BOOT1 вход
const int X0_PIN = PB13;
const int X1_PIN = PB14;
const int X2_PIN = PB11;
const int X3_PIN = PB12;
const int X4_PIN = PE15;
const int X5_PIN = PB10;
const int X6_PIN = PE13;
const int X7_PIN = PE14;
const int POWER_MONITORING_PIN = PC7; // мониторинг питания (делитель напряжения в цепи питания +24В)

/* ------------------- Дискретные выходы Yn ------------------- */
const int RUN_LED_PIN = PD10;
const int Y0_PIN = PC9;
const int Y1_PIN = PC8;
const int Y2_PIN = PA8;
const int Y3_PIN = PA0;
const int Y4_PIN = PB3;
const int Y5_PIN = PD12;

/* ------------------- Аналоговые входы ADn ------------------- */
const int AD0_PIN = PA1;
const int AD1_PIN = PA3;
const int AD2_PIN = PC4;
const int AD3_PIN = PC5;
const int AD4_PIN = PC0;
const int AD5_PIN = PC1;
const int POTENTIOMETER1_PIN = PC2;
const int POTENTIOMETER2_PIN = PC3;

/* ------------------- Аналоговые выходы DAn ------------------- */
const int DA0_PIN = PA4;
const int DA1_PIN = PA5;

/* ------------------- EEPROM на I2C ------------------- */
#define I2C_7BITADDR 0x50 // EEPROM address
// SlowSoftI2CMaster(uint8_t sda, uint8_t scl, bool internal_pullup); // use true as third arg, if you do not have external pullups
SlowSoftI2CMaster si = SlowSoftI2CMaster(PA13, PA15, true);

/* ------------------- RTC ------------------- */
STM32RTC& rtc = STM32RTC::getInstance();

struct Retain {
  int Val1;
  int Val2;
  int Val3;
} retainVals; // Энергонезависимые переменные (читаются/записываются в EEPROM)

bool poweroff;

void readRetainEEPROM() {
  if (!si.i2c_start_wait((I2C_7BITADDR<<1)|I2C_WRITE)) { // init transfer
    si.i2c_stop();
    Serial.println("I2C device busy");
    return;
  }
  si.i2c_write(0); // Загрузка стартового адреса памяти в счетчик, первый байт, для примера 0
  si.i2c_write(0); // Загрузка стартового адреса памяти в счетчик, второй байт, для примера 0
  if (!si.i2c_rep_start((I2C_7BITADDR<<1)|I2C_READ)) { // init transfer
    si.i2c_stop(); // stop communication
    Serial.println("I2C device busy");
    return;
  }
  Serial.print("readRetainEEPROM(); sizeof(retainVals): " + String(sizeof(retainVals))+ "; bytes:");
  byte * pRetainVals = (byte *)&retainVals; // указатель на байты
  for (unsigned i = 0; i < sizeof(retainVals); ++i, ++pRetainVals) { // побайтное чтение из EEPROM начиная со стартового адреса
    *pRetainVals = si.i2c_read(false);
    Serial.print(" ");
    Serial.print(*pRetainVals);
  }
  si.i2c_read(true);
  si.i2c_stop(); // stop communication
  Serial.println();
}

void writeRetainEEPROM() {
  if (!si.i2c_start_wait((I2C_7BITADDR<<1)|I2C_WRITE)) { // init transfer
    si.i2c_stop(); // stop communication
    Serial.println("I2C device busy");
    return;
  }
  si.i2c_write(0); // Загрузка стартового адреса памяти в счетчик, первый байт
  si.i2c_write(0); // Загрузка стартового адреса памяти в счетчик, второй байт
  byte * pRetainVals = (byte *)&retainVals; // указатель на байты
  for (unsigned int i = 0; i < sizeof(retainVals); ++i, ++pRetainVals) { // побайтная запись в EEPROM начиная со стартового адреса
    si.i2c_write(*pRetainVals); // send memory to device
  }
  si.i2c_stop(); // stop communication
  Serial.println("writeRetainEEPROM() completed!");
}

void setup() {
  Serial.setRx(PA10); // Отладка на USART1 вместо HardwareSerial Serial1(PA10, PA9);
  Serial.setTx(PA9);
  // while(!Serial);
  Serial.begin(115200);
  delay(100);
  Serial.println("\nInitialization...");

  /* ------------------- Modbus RTU на RS-485 (Serial4) ------------------- */
  ModbusRTUServer.begin(_RS485, 1, 115200);
  ModbusRTUServer.configureHoldingRegisters(0x00, 10); // configure holding registers at address 0x00
  ModbusRTUServer.configureInputRegisters(0x00, 10); // configure input registers at address 0x00
  
  /* ------------------- Дискретные входы ------------------- */
  pinMode(START_SWITCH_PIN, INPUT_PULLUP); // с внутренней подтяжкой к +3.3В
  pinMode(X0_PIN, INPUT); // Внешняя подтяжка к +3.3В, инвертированная логика
  pinMode(X1_PIN, INPUT);  
  pinMode(X2_PIN, INPUT);
  pinMode(X3_PIN, INPUT);  
  pinMode(X4_PIN, INPUT);
  pinMode(X5_PIN, INPUT);
  pinMode(X6_PIN, INPUT);
  pinMode(X7_PIN, INPUT);
  pinMode(POWER_MONITORING_PIN, INPUT); // мониторинг питания (делитель напряжения в цепи питания +24В)
  
  /* ------------------- Дискретные выходы ------------------- */
  pinMode(RUN_LED_PIN, INPUT); // Сначала инициализируем как вход в высокоимпедансном состоянии
  digitalWrite(RUN_LED_PIN, HIGH); // Инвертированная логика LED, инициализация с HIGH
  pinMode(RUN_LED_PIN, OUTPUT); // Потом переключаем в Output
  pinMode(Y0_PIN, OUTPUT);
  pinMode(Y1_PIN, OUTPUT);
  pinMode(Y2_PIN, OUTPUT);
  pinMode(Y3_PIN, OUTPUT);
  pinMode(Y4_PIN, OUTPUT);
  pinMode(Y5_PIN, OUTPUT);
  
  /* ------------------- Аналоговые входы ADn ------------------- */
  analogReadResolution(12);
  
  /* ------------------- Аналоговые выходы DAn ------------------- */
  analogWriteResolution(12);
  
  /* ------------------- EEPROM на I2C ------------------- */
  if (!si.i2c_init()) {
    Serial.println("I2C init failed");
  }
  readRetainEEPROM(); // Чтение из EEPROM Retain переменных
  
  ModbusRTUServer.holdingRegisterWrite(0, retainVals.Val1); // Копируем Retain в HR Modbus
  ModbusRTUServer.holdingRegisterWrite(1, retainVals.Val2);
  ModbusRTUServer.holdingRegisterWrite(2, retainVals.Val3);
  
  /* ------------------- RTC ------------------- */
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin(); // initialize RTC 24H format
  // Set the time
  /* rtc.setHours(hours); // тут можно установить время
  rtc.setMinutes(minutes);
  rtc.setSeconds(seconds);

  // Set the date
  rtc.setWeekDay(weekDay);
  rtc.setDay(day);
  rtc.setMonth(month);
  rtc.setYear(year);
  */
  // you can use also
  //rtc.setTime(hours, minutes, seconds);
  //rtc.setDate(weekDay, day, month, year);
}

void loop() {
  //if (!digitalRead(X7_PIN)) { // Используем вход X7 как программный reset или переход в загрузчик для прошивки по UART
  if (!digitalRead(START_SWITCH_PIN)) { // Используем вход START_SWITCH_PIN как программный reset или переход в загрузчик для прошивки по UART
    //NVIC_SystemReset(); // Программный reset
    Serial.println("--------------- bootloaderInit() ---------------");
    Serial.flush();
    bootloaderInit(); // Переход в загрузчик
  }
  digitalToggle(RUN_LED_PIN);
  
  /* ------- Мониторинг питания + запись Retain в EEPROM ------- */
  if (!digitalRead(POWER_MONITORING_PIN) && !poweroff) {
    Serial.println("--------------- Power off ---------------");
    poweroff = true;
    retainVals.Val1 = ModbusRTUServer.holdingRegisterRead(0);
    retainVals.Val2 = ModbusRTUServer.holdingRegisterRead(1);
    retainVals.Val3 = ModbusRTUServer.holdingRegisterRead(2);
    writeRetainEEPROM(); // Запиcь Retain в последнии секунды при отключении питания
  }
  if (digitalRead(POWER_MONITORING_PIN) && poweroff) {
    Serial.println("--------------- Power ON ---------------");
    poweroff = false;     
  }
  
  /* ------- Чтение RTC ------- */
  Serial.printf("%02d/%02d/%02d ", rtc.getDay(), rtc.getMonth(), rtc.getYear());
  Serial.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
  
  /* ------- Modbus RTU на RS-485 (Serial4) ------- */
  // int packetReceived = ModbusRTUServer.poll();
  ModbusRTUServer.poll();
  ModbusRTUServer.inputRegisterWrite(0, digitalRead(START_SWITCH_PIN) | digitalRead(START_SWITCH_PIN) << 1); // Input Register 0 - состояние дискретых входов

  digitalWrite(Y0_PIN, !digitalRead(X0_PIN));   
  digitalWrite(Y1_PIN, !digitalRead(X1_PIN));
  digitalWrite(Y2_PIN, !digitalRead(X2_PIN));  
  digitalWrite(Y3_PIN, !digitalRead(X3_PIN));
  //digitalWrite(Y4_PIN, !digitalRead(X4_PIN));
  //digitalWrite(Y5_PIN, !digitalRead(X5_PIN));
  digitalWrite(Y4_PIN, rtc.getSeconds() == 30);
  digitalWrite(Y5_PIN, rtc.getSeconds() == 33);
  //digitalToggle(Y0_PIN);
  //delay(1000);
  //digitalToggle(Y1_PIN);
  //delay(1000);
  //digitalToggle(Y2_PIN);
  //delay(1000);
  //digitalToggle(Y3_PIN);
  //delay(1000);
  //digitalToggle(Y4_PIN);
  //delay(1000);
  //digitalToggle(Y5_PIN); 

  Serial.printf("X0-X7: %d %d %d %d %d %d %d %d\n", !digitalRead(X0_PIN), !digitalRead(X1_PIN), !digitalRead(X2_PIN), !digitalRead(X3_PIN), !digitalRead(X4_PIN), !digitalRead(X5_PIN), !digitalRead(X6_PIN), !digitalRead(X7_PIN));
  Serial.printf("POWER_MONITORING: %d; START_SWITCH_PIN: %d\n", digitalRead(POWER_MONITORING_PIN), digitalRead(START_SWITCH_PIN));
  Serial.printf("AD0-AD5: %d %d %d %d %d %d\n", analogRead(AD0_PIN), analogRead(AD1_PIN), analogRead(AD2_PIN), analogRead(AD3_PIN), analogRead(AD4_PIN), analogRead(AD5_PIN));
  Serial.printf("POTENTIOMETER 1-2: %d %d\n", analogRead(POTENTIOMETER1_PIN), analogRead(POTENTIOMETER2_PIN));

  int DA0Voltage = ModbusRTUServer.holdingRegisterRead(0); // 0-10
  int DA1Voltage = ModbusRTUServer.holdingRegisterRead(1); // 0-10
  analogWrite(DA0_PIN, map(DA0Voltage, 0, 10, 0, 4095)); // 12 bit DAC
  analogWrite(DA1_PIN, map(DA1Voltage, 0, 10, 0, 4095)); // 12 bit DAC
  Serial.printf("DA0-DA1: %d %d\n", map(DA0Voltage, 0, 10, 0, 4095), map(DA1Voltage, 0, 10, 0, 4095));

  delay(analogRead(POTENTIOMETER1_PIN)/4); // Период цикла от положения потенциометра 1
}
