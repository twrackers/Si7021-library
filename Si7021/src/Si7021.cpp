#include <Wire.h>

#include "Si7021.h"

const byte led = 13;

Si7021::Si7021(const uint16_t interval) : 
  StateMachine(1, true), 
  m_state(eInitial),
  m_delay(0L), 
  m_interval(interval * 1000),
  m_sample(0L),
  m_rh(0.0),
  m_temp(0.0),
  m_fault(false)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

bool Si7021::update()
{
  // Return now if it's not time to update.
  if (!StateMachine::update()) return false;
  
  // Get the current clock count.
  uint32_t now = millis();
  
  if (m_state == eInitial) {
    
    // Send the reset command.
    writeCommand(eReset);
    // Set 50 msec delay.
    m_delay = now + 50;
    m_state = eResetting;
    
  } else if (m_state == eResetting) {
    
    // If delay has expired...
    if (now >= m_delay) {
      
      // Verify status register has been reset.
      uint8_t status = readRegister8(eReadRHTReg);
      // Verify status register is reset.
      if (status != 0x3A) {
        // If not, enter fault state.
        digitalWrite(led, HIGH);
        m_sample = now + 50;
        m_fault = true;
        m_state = eFault;
      } else {
        // Otherwise, ready for normal operation.
        digitalWrite(led, LOW);
        m_sample = now + m_interval;
        m_fault = false;
        m_state = eReady;
      }
      
    }
    
  } else if (m_state == eReady) {
    
    // If it's time to sample...
    if (now >= m_sample) {
      // ... send command to read RH.
      writeCommand(eMeasRH_NoHold, false);
      // Set 25 msec delay.
      m_delay = now + 25;
      m_state = eReading;
      // Turn on LED during delay.
      digitalWrite(13, HIGH);
    }
    
  } else if (m_state == eReading) {
    
    // If delay has expired...
    if (now >= m_delay) {
      
      // Request 3 bytes of RH data and checksum (not used).
      Wire.requestFrom(Si7021_ADDR, 3);
      // Raw RH is first 2 bytes (high then low).
      uint16_t rh = Wire.read();
      rh = (rh << 8) | Wire.read();
      // Checksum is not used.
      uint8_t cksum = Wire.read();
      // Compute RH.
      m_rh = constrain(
        (((float) rh * 125.0) / 65536.0 - 6.0),
        0.0, 
        100.0
      );
      // Send command to fetch temperature after RH sample.
      // No conversion time required.
      uint16_t temp = readRegister16(eReadPrevTemp);
      m_temp = ((float) temp * 175.72) / 65536.0 - 46.85;
      // Set time for next sample.
      m_sample = now + m_interval;
      m_state = eReady;
      // Turn off LED upon completion.
      digitalWrite(13, LOW);
      
    }
      
  } else /* if (m_state == eFault) */ {
      
    // If we're in a fault state, wait and then try to reset sensor.
    // m_fault will remain true until reset succeeds.
    if (now >= m_sample) {
      m_state = eInitial;
    }
    
  }
  
  return true;
}

void Si7021::begin()
{
  Wire.begin();
}

float Si7021::getTemp() const
{
  return m_temp;
}

float Si7021::getRH() const
{
  return m_rh;
}

bool Si7021::isFault() const
{
  return m_fault;
}

void Si7021::reset()
{
  m_state = eInitial;
}

// Private methods

uint8_t Si7021::writeCommand(E_CMDS cmd, const bool release)
{
  Wire.beginTransmission(Si7021_ADDR);
  Wire.write((uint8_t) cmd);
  return Wire.endTransmission(release);
}

uint8_t Si7021::writeRegister8(E_CMDS reg, const uint8_t value) 
{
  Wire.beginTransmission(Si7021_ADDR);
  Wire.write((uint8_t)reg);
  Wire.write(value);
  return Wire.endTransmission();
}

uint8_t Si7021::readRegister8(E_CMDS reg) 
{
  writeCommand(reg, false);
  Wire.requestFrom(Si7021_ADDR, 1);
  return Wire.read();
}

uint16_t Si7021::readRegister16(E_CMDS reg) 
{
  writeCommand(reg, false);
  Wire.requestFrom(Si7021_ADDR, 2);
  uint16_t value = Wire.read();
  return (value << 8) | Wire.read();
}
