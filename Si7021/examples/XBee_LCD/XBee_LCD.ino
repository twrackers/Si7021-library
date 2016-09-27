#include <RingBuffer.h>
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
#include <SoftwareSerial.h>

const byte ROWS = 2;
const byte COLS = 16;

SoftwareSerial mySerial(4, 5);
Adafruit_LiquidCrystal lcd(0);
RingBuffer rb(20);

void setup()
{
  // set up the LCD's number of rows and columns: 
  lcd.begin(COLS, ROWS);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  // Define degree symbol.
  byte ch[] = { 0x0C, 0x12, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00 };
  lcd.createChar(1, ch);
}

void loop()
{
  if (mySerial.available()) {
    byte c = mySerial.read();
    if (c == '\n') {
      // ignore
    } else if (c == '\r') {
      byte buf[COLS + 1];
      byte* p = buf;
      while (!rb.isEmpty()) {
        *p++ = rb.pop();
      }
      *p = '\0';
      String s((char*) buf);
      int idx = s.indexOf(',');
      String rhString = s.substring(0, idx);
      String tempString = s.substring(idx + 1);
      float rh = rhString.toFloat();
      float temp = tempString.toFloat();
      String rhText   = String("RH:   ") + String(rh, 2) + String(" %");
      String tempText = String("Temp: ") + String(temp, 2) + String(' ')
        + String('\001') + String('C');
      lcd.setCursor(0, 0);
      lcd.print(rhText.c_str());
      lcd.setCursor(0, 1);
      lcd.print(tempText.c_str());
    } else {
      rb.push(c);
    }
  }
}
