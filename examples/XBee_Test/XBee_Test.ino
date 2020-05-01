#include <Pulse.h>
#include <StateMachine.h>
#include <Streaming.h>
#include <Si7021.h>

Si7021 sensor(15);
StateMachine pace(15000, true);

void setup() 
{
  Serial.begin(9600);
  sensor.begin();
}

void loop()
{
  sensor.update();
  if (pace.update()) {
    Serial << sensor.getRH() << "," << sensor.getTemp() << endl;
  }
}

