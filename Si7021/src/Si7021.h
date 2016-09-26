#ifndef _SI7021__H_
#define _SI7021__H_

#include <Arduino.h>
#include <StateMachine.h>

class Si7021 : public StateMachine
{
private:
  const uint8_t Si7021_ADDR = 0x40;
  
  enum E_CMDS {
    eMeasRH_Hold     = 0xE5,
    eMeasRH_NoHold   = 0xF5,
    eMeasTemp_Hold   = 0xE3,
    eMeasTemp_NoHold = 0xF3,
    eReadPrevTemp    = 0xE0,
    eReset           = 0xFE,
    eWriteRHTReg     = 0xE6,
    eReadRHTReg      = 0xE7,
    eWriteHeaterReg  = 0x51,
    eReadHeaterReg   = 0x11
  };
  
  enum E_STATE {
    eInitial,
    eResetting,
    eReady,
    eReading,
    eFault
  } m_state;
  
  uint32_t m_delay;
  uint32_t m_interval;
  uint32_t m_sample;
  float    m_rh;
  float    m_temp;
  bool     m_fault;
  
  uint8_t writeCommand(E_CMDS cmd, const bool release = true);
  uint8_t writeRegister8(E_CMDS reg, const uint8_t value);
  uint8_t readRegister8(E_CMDS reg);
  uint16_t readRegister16(E_CMDS reg);

public:
  Si7021(const uint16_t interval);
  
  virtual bool update();
  
  float getTemp() const;
  float getRH() const;
  bool isFault() const;
  void reset();
};

#endif
