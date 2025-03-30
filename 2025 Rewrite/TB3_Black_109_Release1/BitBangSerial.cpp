#include "BitBangSerial.hpp"

#define TXPIN (1 << PG5)   // Bit mask for Pin 4
#define TXPORT PORTG       // Define the port for Pin 4
#define TXDDR DDRG         // Define the data direction register for Pin 4
constexpr uint32_t BAUD_RATE = 115200;    // Baud Rate
constexpr uint16_t BIT_DELAY = (F_CPU / BAUD_RATE / 4) - (15 / 4); // Compute bit delay
constexpr uint16_t START_BIT_DELAY = BIT_DELAY + 1; // Compute bit delay
constexpr uint16_t STOP_BIT_DELAY = (BAUD_RATE == 115200) ? static_cast<uint16_t>(BIT_DELAY * 6.65f) : BIT_DELAY;

BitBangSerial::BitBangSerial(uint8_t txPin, uint32_t speed) {
  TXDDR |= TXPIN;          // Set pin 4 as an output pin
  TXPORT |= TXPIN;         // Set pin 4 to high (idle state)
}

size_t BitBangSerial::write(uint8_t value) {
    uint8_t oldSREG = SREG;  // Save global interrupt flag
    cli();                   // Disable interrupts

    TXPORT &= ~TXPIN;        // Start bit
    _delay_loop_2(START_BIT_DELAY);

    for (uint8_t bitcount = 0; bitcount < 8; bitcount++) {
        if (value & 0x01)  TXPORT |= TXPIN;
        else               TXPORT &= ~TXPIN;
        
        value >>= 1;
        _delay_loop_2(BIT_DELAY);
    }

    TXPORT |= TXPIN;         // Stop bit
    _delay_loop_2(STOP_BIT_DELAY);
    SREG = oldSREG;          // Restore global interrupt flag
    return 1;                // Always return 1 byte written
}