#pragma once

#include <stdint.h>

class SerialHandler
{
public:

    SerialHandler();
    uint8_t initialize();
    uint8_t write(const uint8_t);
    uint8_t write(const uint8_t txData[], uint8_t txDataLength);
    uint8_t read(uint8_t&);
    size_t readBytes( uint8_t *buffer, size_t length);
    void begin(unsigned long baudrate, uint16_t config);
    void begin(unsigned long baudrate);
    uint8_t available();

    void end();
    
    void enableRs485Mode(void);
    void disableRs485Mode(void);
    
private:   
    uint8_t m_rs485ModeEnable;
    void enableRx();
    void enableTx();
};
