#pragma once
#include <cstdint>
#include <Arduino.h>

class Blink {
    uint8_t pin = 2; // Brug GPIO 2 (indbygget LED på de fleste ESP32 DevKit)

    // ESP32 PWM-kanal
    uint8_t channel = 0;
    uint8_t resolution = 8;  // 8-bit opløsning
    uint32_t freq = 5000;    // 5 kHz frekvens

public:
    Blink() {
        // Konfigurer pin som output
        pinMode(pin, OUTPUT);

        // Konfigurer PWM kanal på ESP32
        ledcSetup(channel, freq, resolution);
        ledcAttachPin(pin, channel);
    }

    // Skaber en pulserende effekt ved brug af PWM
    void pulse() {
        for (int i = 0; i < 256; i++) {
            ledcWrite(channel, i);  // Styr lysstyrken med PWM
            delay(3);
        }
        for (int i = 255; i >= 0; i--) {
            ledcWrite(channel, i);
            delay(3);
        }
    }

    // Hurtig blinkfunktion
    void fastBlink(int times) {
        for (int i = 0; i < times; i++) {
            ledcWrite(channel, 255);
            delay(100);
            ledcWrite(channel, 0);
            delay(100);
        }
    }

    void turnOn() {
        ledcWrite(channel, 255);  // Maksimal lysstyrke
    }

    void turnOff() {
        ledcWrite(channel, 0);  // Sluk lyset
    }
};
