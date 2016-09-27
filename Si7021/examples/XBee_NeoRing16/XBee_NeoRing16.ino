#include <Streaming.h>
#include <Pulser.h>
#include <RingBuffer.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

SoftwareSerial mySerial(4, 5);
RingBuffer rb(64);
Pulser pulser(3000, 3000);
float rh;
float temp;

class Ring
{
private:
  Adafruit_NeoPixel& m_pixels;
  const byte m_numPixels;
  const byte dim_green = 3;
  
public:
  Ring(Adafruit_NeoPixel& pixels, const byte numPixels) :
    m_pixels(pixels), m_numPixels(numPixels)
  {
    for (byte i = 0; i < m_numPixels; ++i) {
      m_pixels.setPixelColor(i, m_pixels.Color(0, dim_green, 0));
    }
  }

  void setValue(const bool isTemp, const byte val)
  {
    byte step = 256 / m_numPixels;
    byte w = val / step;
    byte f = (val % step) * step;
    uint32_t c1 = isTemp
      ? m_pixels.Color(255, dim_green, 0) 
      : m_pixels.Color(0, dim_green, 255);
    uint32_t c2 = isTemp
      ? m_pixels.Color(f, dim_green, 0)
      : m_pixels.Color(0, dim_green, f);
    for (int i = 0; i < w; ++i) {
      m_pixels.setPixelColor(m_numPixels - 1 - i, c1);
    }
    m_pixels.setPixelColor(m_numPixels - 1 - w, c2);
    for (int i = (w + 1); i < m_numPixels; ++i) {
      m_pixels.setPixelColor(m_numPixels - 1 - i, m_pixels.Color(0, dim_green, 0));
    }
    m_pixels.show();
  }
};

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Ring ring(pixels, NUMPIXELS);

void parseFromBuffer(RingBuffer& rb, float& rh, float& temp)
{
  byte buf[33];
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
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup()
{
  // Start interface to NeoPixels.
  pixels.begin();
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop()
{
  bool newData = false;
  while (mySerial.available()) {
    byte c = mySerial.read();
    if (c == '\n') {
      // ignore
    } else if (c == '\r') {
      parseFromBuffer(rb, rh, temp);
      newData = true;
    } else {
      if (isprint(c)) {
        rb.push(c);
      }
    }
  }
  pulser.update();
  if (newData) {
    char c1, c2, c3, c4;
    int val;
    bool toggle = pulser.read();
    if (toggle) {
      val = (int) fmap(temp, 20.0, 40.0, 0.0, 255.0);
      ring.setValue(true, val);
    } else {
      rh = constrain(rh, 0.0, 100.0);
      val = (int) fmap(rh, 0.0, 100.0, 0.0, 255.0);
      ring.setValue(false, val);
    }
    newData = false;
  }
}
