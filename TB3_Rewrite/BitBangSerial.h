#ifndef BITBANGSERIAL_H
#define BITBANGSERIAL_H

#include <Arduino.h>

constexpr uint32_t BAUD_RATE = 9600;
constexpr uint8_t WRITE_BUF_SIZE = 64;
constexpr uint32_t BOUNCE_MICROS = 20;  // delay between commands

class BitBangSerial {
  public:
    BitBangSerial(uint8_t pin);
    void begin();
    void end();
    bool write(uint8_t b);
    uint32_t handle();
    bool isBusy() const;
    uint8_t availableSpace() const;

  private:
    uint8_t _pin;
    uint32_t _lastBitTime = 0;
    const uint32_t _bitTime = 1000000 / BAUD_RATE;
    uint32_t _nextWriteTime = 0;
    int8_t _bitIndex = -1;
    uint8_t _buffer[WRITE_BUF_SIZE];
    uint8_t _bufferReadPos = 0;
    uint8_t _bufferWritePos = 0;
};

#endif  // BITBANGSERIAL_H
