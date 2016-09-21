#include <Si7021.h>
#include <Pulse.h>
#include <StateMachine.h>
#include <Streaming.h>

Pulse pulse(13, HIGH, 100);
StateMachine pace(1000, true);
Si7021 rht(3);  // 3 second sample interval

void setup()
{
  Serial.begin(115200);
  rht.begin();
}

void loop()
{
  rht.update();
  if (pace.update()) {
    pulse.trigger();
    Serial << rht.getRH() << "," << rht.getTemp() << endl;
  }
  pulse.update();
}

