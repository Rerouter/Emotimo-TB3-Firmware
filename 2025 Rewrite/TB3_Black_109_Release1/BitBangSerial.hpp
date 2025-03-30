#ifndef BITBANGSERIAL_HPP
#define BITBANGSERIAL_HPP

#include "Arduino.h"
#include <Stream.h> // Include the Stream class header

class BitBangSerial : public Stream {
public:
    BitBangSerial(uint8_t pin, uint32_t speed);
    size_t write(uint8_t c) override; // Override for the Stream write method
    using Print::write; // Pull in other overloads of write
    int available() override { return 0; } // Required override for Stream
    int read() override { return -1; } // Required override for Stream
    int peek() override { return -1; } // Required override for Stream

private:
};

#endif // BITBANGSERIAL_HPP
