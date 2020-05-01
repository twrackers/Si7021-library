#include <Pulser.h>
#include <RingBuffer.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

SoftwareSerial mySerial(4, 5);
RingBuffer rb(20);
Adafruit_AlphaNum4 alpha4;
Pulser pulser(3000, 3000);
float rh;
float temp;
bool newData = false;

void setup()
{
  // Start I2C interface to LED display.
  alpha4.begin(0x70);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop()
{
  if (mySerial.available()) {
    byte c = mySerial.read();
    if (c == '\n') {
      // ignore
    } else if (c == '\r') {
      byte buf[20];
      byte* p = buf;
      while (!rb.isEmpty()) {
        *p++ = rb.pop();
      }
      *p = '\0';
      String s((char*) buf);
      int idx = s.indexOf(',');
      String rhString = s.substring(0, idx);
      String tempString = s.substring(idx + 1);
      rh = rhString.toFloat();
      temp = tempString.toFloat();
      newData = true;
    } else {
      rb.push(c);
    }
  }
  pulser.update();
  if (newData) {
    char c1, c2, c3, c4;
    int val;
    bool toggle = pulser.read();
    if (toggle) {
      val = (int) (temp + 0.5);
    } else {
      val = (int) (constrain(rh, 0.0, 100.0) + 0.5);
    }
    bool lz = true;
    c1 = '0' + (val / 100);
    if (lz && c1 == '0') {
      c1 = ' ';
    } else {
      lz = false;
    }
    val %= 100;
    c2 = '0' + (val / 10);
    if (lz && c2 == '0') {
      c2 = ' ';
    } else {
      lz = false;
    }
    val %= 10;
    c3 = '0' + val;
    c4 = toggle ? 'C' : '%';
    alpha4.writeDigitAscii(0, c1);
    alpha4.writeDigitAscii(1, c2);
    alpha4.writeDigitAscii(2, c3);
    alpha4.writeDigitAscii(3, c4);
    alpha4.writeDisplay();
    newData = false;
  }
}
