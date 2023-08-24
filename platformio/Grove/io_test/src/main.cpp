/*
 * main.cpp
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>
#include <Wire.h>

////////////////////////////////////////////////////////////////////////////////
// Constants

static constexpr uint8_t GROVE_SUPPLY_PIN   = PIN_POWER_SUPPLY_GROVE;
static constexpr uint8_t LED_PIN            = LED_BUILTIN;
static constexpr uint8_t BUTTON_PIN         = PIN_BUTTON1;
static constexpr uint8_t GROVE_D0_P1_PIN    = D0;
static constexpr uint8_t GROVE_D0_P2_PIN    = D1;
static constexpr uint8_t GROVE_D1_P1_PIN    = D2;
static constexpr uint8_t GROVE_D1_P2_PIN    = D3;
static constexpr uint8_t GROVE_D2_P1_PIN    = D4;
static constexpr uint8_t GROVE_D2_P2_PIN    = D5;
static constexpr uint8_t GROVE_A0_P1_PIN    = D6;
static constexpr uint8_t GROVE_A0_P2_PIN    = D7;
static constexpr uint8_t GROVE_UART_RXD_PIN = D8;
static constexpr uint8_t GROVE_UART_TXD_PIN = D9;
static constexpr uint8_t GROVE_I2C_SCL_PIN  = D10;
static constexpr uint8_t GROVE_I2C_SDA_PIN  = D11;

static constexpr uint32_t INTERVAL = 200;   // [msec.]

////////////////////////////////////////////////////////////////////////////////
// test functions

static void TestDigitalRead()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(GROVE_D0_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D0_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D1_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D1_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D2_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D2_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_A0_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_A0_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_UART_RXD_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_UART_TXD_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_I2C_SCL_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_I2C_SDA_PIN, INPUT_PULLDOWN);

    for (;;)
    {
        printf("BTN:");
        printf(digitalRead(BUTTON_PIN) == LOW ? "*" : ".");
        printf(" D0:");
        printf(digitalRead(GROVE_D0_P1_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_D0_P2_PIN) == HIGH ? "*" : ".");
        printf(" D1:");
        printf(digitalRead(GROVE_D1_P1_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_D1_P2_PIN) == HIGH ? "*" : ".");
        printf(" D2:");
        printf(digitalRead(GROVE_D2_P1_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_D2_P2_PIN) == HIGH ? "*" : ".");
        printf(" A0:");
        printf(digitalRead(GROVE_A0_P1_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_A0_P2_PIN) == HIGH ? "*" : ".");
        printf(" UART:");
        printf(digitalRead(GROVE_UART_RXD_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_UART_TXD_PIN) == HIGH ? "*" : ".");
        printf(" I2C:");
        printf(digitalRead(GROVE_I2C_SCL_PIN) == HIGH ? "*" : ".");
        printf(digitalRead(GROVE_I2C_SDA_PIN) == HIGH ? "*" : ".");
        printf("\n");

        delay(INTERVAL);
    }
}

static void ButtonHandler() { printf("Button\n"); }
static void GroveD0P1Handler() { printf("GroveD0P1\n"); }
static void GroveD0P2Handler() { printf("GroveD0P2\n"); }
static void GroveD1P1Handler() { printf("GroveD1P1\n"); }
static void GroveD1P2Handler() { printf("GroveD1P2\n"); }
static void GroveD2P1Handler() { printf("GroveD2P1\n"); }
static void GroveD2P2Handler() { printf("GroveD2P2\n"); }
static void GroveA0P1Handler() { printf("GroveA0P1\n"); }
static void GroveA0P2Handler() { printf("GroveA0P2\n"); }
static void GroveUartP1Handler() { printf("GroveUartP1\n"); }
static void GroveUartP2Handler() { printf("GroveUartP2\n"); }
static void GroveI2cP1Handler() { printf("GroveI2cP1\n"); }
static void GroveI2cP2Handler() { printf("GroveI2cP2\n"); }

static void TestInterrupt()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(GROVE_D0_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D0_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D1_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D1_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D2_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_D2_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_A0_P1_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_A0_P2_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_UART_RXD_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_UART_TXD_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_I2C_SCL_PIN, INPUT_PULLDOWN);
    pinMode(GROVE_I2C_SDA_PIN, INPUT_PULLDOWN);

    attachInterrupt(BUTTON_PIN, ButtonHandler, FALLING);
    attachInterrupt(GROVE_D0_P1_PIN, GroveD0P1Handler, RISING);
    attachInterrupt(GROVE_D0_P2_PIN, GroveD0P2Handler, RISING);
    attachInterrupt(GROVE_D1_P1_PIN, GroveD1P1Handler, RISING);
    attachInterrupt(GROVE_D1_P2_PIN, GroveD1P2Handler, RISING);
    attachInterrupt(GROVE_D2_P1_PIN, GroveD2P1Handler, RISING);
    attachInterrupt(GROVE_D2_P2_PIN, GroveD2P2Handler, RISING);
    // attachInterrupt(GROVE_A0_P1_PIN, GroveA0P1Handler, RISING);
    // attachInterrupt(GROVE_A0_P2_PIN, GroveA0P2Handler, RISING);
    // attachInterrupt(GROVE_UART_RXD_PIN, GroveUartP1Handler, RISING);
    // attachInterrupt(GROVE_UART_TXD_PIN, GroveUartP2Handler, RISING);
    // attachInterrupt(GROVE_I2C_SCL_PIN, GroveI2cP1Handler, RISING);
    // attachInterrupt(GROVE_I2C_SDA_PIN, GroveI2cP2Handler, RISING);

    for (;;)
    {
        delay(INTERVAL);
    }
}

static void TestDigitalWrite()
{
    digitalWrite(GROVE_SUPPLY_PIN, LOW);
    pinMode(GROVE_SUPPLY_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(GROVE_D0_P1_PIN, LOW);
    pinMode(GROVE_D0_P1_PIN, OUTPUT);
    digitalWrite(GROVE_D0_P2_PIN, LOW);
    pinMode(GROVE_D0_P2_PIN, OUTPUT);
    digitalWrite(GROVE_D1_P1_PIN, LOW);
    pinMode(GROVE_D1_P1_PIN, OUTPUT);
    digitalWrite(GROVE_D1_P2_PIN, LOW);
    pinMode(GROVE_D1_P2_PIN, OUTPUT);
    digitalWrite(GROVE_D2_P1_PIN, LOW);
    pinMode(GROVE_D2_P1_PIN, OUTPUT);
    digitalWrite(GROVE_D2_P2_PIN, LOW);
    pinMode(GROVE_D2_P2_PIN, OUTPUT);
    digitalWrite(GROVE_A0_P1_PIN, LOW);
    pinMode(GROVE_A0_P1_PIN, OUTPUT);
    digitalWrite(GROVE_A0_P2_PIN, LOW);
    pinMode(GROVE_A0_P2_PIN, OUTPUT);
    digitalWrite(GROVE_UART_RXD_PIN, LOW);
    pinMode(GROVE_UART_RXD_PIN, OUTPUT);
    digitalWrite(GROVE_UART_TXD_PIN, LOW);
    pinMode(GROVE_UART_TXD_PIN, OUTPUT);
    digitalWrite(GROVE_I2C_SCL_PIN, LOW);
    pinMode(GROVE_I2C_SCL_PIN, OUTPUT);
    digitalWrite(GROVE_I2C_SDA_PIN, LOW);
    pinMode(GROVE_I2C_SDA_PIN, OUTPUT);

    for (;;)
    {
        digitalWrite(GROVE_SUPPLY_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        digitalWrite(LED_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_D0_P1_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_D0_P2_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_D1_P1_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_D1_P2_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_D2_P1_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_D2_P2_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_A0_P1_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_A0_P2_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_UART_RXD_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_UART_TXD_PIN, millis() % 1000 < 800 ? HIGH : LOW);
        // digitalWrite(GROVE_I2C_SCL_PIN, millis() % 1000 < 200 ? HIGH : LOW);
        // digitalWrite(GROVE_I2C_SDA_PIN, millis() % 1000 < 800 ? HIGH : LOW);

        delay(INTERVAL);
    }
}

static void TestPwm()
{
    static constexpr uint8_t P1_PIN = LED_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_D0_P1_PIN;
    static constexpr uint8_t P2_PIN = GROVE_D0_P2_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_D1_P1_PIN;
    // static constexpr uint8_t P2_PIN = GROVE_D1_P2_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_D2_P1_PIN;
    // static constexpr uint8_t P2_PIN = GROVE_D2_P2_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_A0_P1_PIN;
    // static constexpr uint8_t P2_PIN = GROVE_A0_P2_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_UART_RXD_PIN;
    // static constexpr uint8_t P2_PIN = GROVE_UART_TXD_PIN;
    // static constexpr uint8_t P1_PIN = GROVE_I2C_SCL_PIN;
    // static constexpr uint8_t P2_PIN = GROVE_I2C_SDA_PIN;

    HwPWM0.addPin(P1_PIN);
    HwPWM0.addPin(P2_PIN);
    HwPWM0.begin();
    HwPWM0.setResolution(15);
    HwPWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_1);

    for (;;)
    {
        for (unsigned i = 0; i < bit(15); i += 1024)
        {
            HwPWM0.writePin(P1_PIN, i);
            HwPWM0.writePin(P2_PIN, bit(15) - 1 - i);
            delay(20);
        }

        delay(INTERVAL);
    }
}

static void TestAd()
{
    for (;;)
    {
        printf(" A0:");
        printf("%lu ", analogRead(A0));
        printf("%lu ", analogRead(A1));
        printf(" D2:");
        printf("%lu ", analogRead(A2));
        printf("%lu ", analogRead(A3));
        printf(" I2C:");
        printf("%lu ", analogRead(A4));
        printf("%lu ", analogRead(A5));
        printf("\n");

        delay(INTERVAL);
    }
}

static void TestI2c()
{
    Wire.begin();
    Wire.setClock(100000);

    Wire.beginTransmission(0x53);
    Wire.write(0x31);
    Wire.write(0x0b);
    Wire.endTransmission();

    Wire.beginTransmission(0x53);
    Wire.write(0x2d);
    Wire.write(0x08);
    Wire.endTransmission();

    for (;;)
    {
        uint8_t dac[6];
        uint16_t x, y, z;
        float X_Axis, Y_Axis, Z_Axis;

        Wire.beginTransmission(0x53);
        Wire.write(0x32);
        Wire.endTransmission();

        Wire.requestFrom(0x53, 6);
        for (int i = 0; i < 6; ++i)
        {
            while (Wire.available() == 0){}
            dac[i] = Wire.read();
        }

        x = (dac[1] << 8) | dac[0];
        X_Axis = float(*(int16_t*)&x) * 0.0392266;
        y = (dac[3] << 8) | dac[2];
        Y_Axis = float(*(int16_t*)&y) * 0.0392266;
        z = (dac[5] << 8) | dac[4];
        Z_Axis = float(*(int16_t*)&z) * 0.0392266;

        printf("ADXL345 %f %f %f\n", X_Axis, Y_Axis, Z_Axis);

        delay(INTERVAL);
    }
}

static void TestUart()
{
    Serial1.begin(115200);

    for (;;)
    {
        while (Serial1.available() >= 1)
        {
            Serial1.write(Serial1.read());
        }

        delay(INTERVAL);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

extern "C" void setup()
{
    // TestDigitalRead();
    // TestInterrupt();
    TestDigitalWrite();
    // TestPwm();
    // TestAd();
    // TestI2c();
    // TestUart();

    suspendLoop();
}

extern "C" void loop()
{
}

////////////////////////////////////////////////////////////////////////////////
