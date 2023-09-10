#include <Arduino.h>
#include "BitBangSerial.h"

BitBangSerial::BitBangSerial(uint8_t pin) : _pin(pin) {}

void BitBangSerial::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);
}

void BitBangSerial::end() {
  digitalWrite(_pin, HIGH);
  pinMode(_pin, INPUT);
}

bool BitBangSerial::write(uint8_t b) {
  uint8_t nextWritePos = (_bufferWritePos + 1) % WRITE_BUF_SIZE;
  if (nextWritePos == _bufferReadPos) {
    // Buffer is full
    return false;
  }
  _buffer[_bufferWritePos] = b;
  _bufferWritePos = nextWritePos;
  _nextWriteTime = micros() + BOUNCE_MICROS;  // Add delay between bytes
  return true;
}

uint32_t BitBangSerial::handle() {
  uint32_t now = micros();

  if (_bitIndex == -1) {
      // start bit
      digitalWrite(_pin, LOW);
  } else if (_bitIndex >= 0 && _bitIndex <= 7) {
      // data bits
      digitalWrite(_pin, _buffer[_bufferReadPos] & (1 << _bitIndex) ? HIGH : LOW);
  } else if (_bitIndex == 8) {
      // stop bit
      digitalWrite(_pin, HIGH);
  } else {
      // finished with this byte
      _bufferReadPos = (_bufferReadPos + 1) % WRITE_BUF_SIZE;
      _bitIndex = -2;
      if (!isBusy()) {
          // No more bytes to send; return 0
          return 0;
      }
  }
  _bitIndex++;
  _lastBitTime = now;

  // Return the timestamp of the next expected call
  return max(_lastBitTime + _bitTime, 1);
}

bool BitBangSerial::isBusy() const {
  return _bufferReadPos != _bufferWritePos;
}

uint8_t BitBangSerial::availableSpace() const {
    if (_bufferWritePos >= _bufferReadPos) {
        return WRITE_BUF_SIZE - (_bufferWritePos - _bufferReadPos);
    } else {
        return _bufferReadPos - _bufferWritePos;
    }
}
