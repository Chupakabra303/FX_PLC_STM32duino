// MCU STM32F103VCT6

#include "..\STM32duinoBootloaderInit.h"

// Serial1 в этом проекте используется для отладки. См. setup();
// HardwareSerial Serial1(PA10, PA9); // HardwareSerial(PinName _rx, PinName _tx, PinName _rts = NC, PinName _cts = NC);

/* ------------------- RS-485 (Serial4) ------------------- */
HardwareSerial Serial4(PC11, PC10);
const int DE_PIN = PA14; // RS-485 DE (driver enable)

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

bool poweroff;

void setup() {
  Serial.setRx(PA10); // Отладка на USART1 вместо HardwareSerial Serial1(PA10, PA9);
  Serial.setTx(PA9);
  // while(!Serial);
  Serial.begin(115200);
  Serial.println("\nInitialization...");

  /* ------------------- RS-485 (Serial4) ------------------- */
  Serial4.begin(115200);
  pinMode(DE_PIN, OUTPUT);// RS-485 DE (driver enable)
  digitalWrite(DE_PIN, HIGH);
  Serial4.println("\nRS-485 Initialization...");
  
  /* ------------------- Дискретные входы ------------------- */
  pinMode(START_SWITCH_PIN, INPUT_PULLUP); // с внутренней подтяжкой к +3.3В
  pinMode(X0_PIN, INPUT);
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
}

void loop() {
  //if (!digitalRead(X7_PIN)) { // Используем вход X7 как программный reset или переход в загрузчик для прошивки по UART
  if (!digitalRead(START_SWITCH_PIN)) { // Используем вход START_SWITCH_PIN как программный reset или переход в загрузчик для прошивки по UART
    //NVIC_SystemReset(); // Программный reset
    Serial.println("--------------- bootloaderInit() ---------------");
    Serial4.println("RS-485 --------------- bootloaderInit() ---------------");
    Serial.flush();
    Serial4.flush();
    bootloaderInit(); // Переход в загрузчик
  }
  digitalToggle(RUN_LED_PIN);
  
  /* ------- Мониторинг питания ------- */
  if (!digitalRead(POWER_MONITORING_PIN) && !poweroff) {
    Serial.println("--------------- Power OFF ---------------");
    Serial4.println("RS-485 --------------- Power OFF ---------------");
    poweroff = true;
  }
  if (digitalRead(POWER_MONITORING_PIN) && poweroff) {
    Serial.println("--------------- Power ON ---------------");
    Serial4.println("RS-485 --------------- Power ON ---------------");
    poweroff = false;    
  }
 
  digitalWrite(Y0_PIN, !digitalRead(X0_PIN));   
  digitalWrite(Y1_PIN, !digitalRead(X1_PIN));
  digitalWrite(Y2_PIN, !digitalRead(X2_PIN));  
  digitalWrite(Y3_PIN, !digitalRead(X3_PIN));
  digitalWrite(Y4_PIN, !digitalRead(X4_PIN));
  digitalWrite(Y5_PIN, !digitalRead(X5_PIN));
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

  int DA0Voltage = 5; // 0-10
  int DA1Voltage = 1; // 0-10
  analogWrite(DA0_PIN, map(DA0Voltage, 0, 10, 0, 4095)); // 12 bit DAC
  analogWrite(DA1_PIN, map(DA1Voltage, 0, 10, 0, 4095)); // 12 bit DAC
  Serial.printf("DA0-DA1: %d %d\n", map(DA0Voltage, 0, 10, 0, 4095), map(DA1Voltage, 0, 10, 0, 4095));

  /* ------- RS-485 (Serial4) ------- */
  Serial4.printf("RS-485 X0-X7: %d %d %d %d %d %d %d %d\n", !digitalRead(X0_PIN), !digitalRead(X1_PIN), !digitalRead(X2_PIN), !digitalRead(X3_PIN), !digitalRead(X4_PIN), !digitalRead(X5_PIN), !digitalRead(X6_PIN), !digitalRead(X7_PIN));
  Serial4.printf("RS-485 POWER_MONITORING: %d; START_SWITCH_PIN: %d\n", digitalRead(POWER_MONITORING_PIN), digitalRead(START_SWITCH_PIN));
  Serial4.printf("RS-485 AD0-AD5: %d %d %d %d %d %d\n", analogRead(AD0_PIN), analogRead(AD1_PIN), analogRead(AD2_PIN), analogRead(AD3_PIN), analogRead(AD4_PIN), analogRead(AD5_PIN));
  Serial4.printf("RS-485 POTENTIOMETER 1-2: %d %d\n", analogRead(POTENTIOMETER1_PIN), analogRead(POTENTIOMETER2_PIN));
  Serial4.printf("RS-485 DA0-DA1: %d %d\n", map(DA0Voltage, 0, 10, 0, 4095), map(DA1Voltage, 0, 10, 0, 4095));

  delay(analogRead(POTENTIOMETER1_PIN)/4); // Период цикла от положения потенциометра 1
}
