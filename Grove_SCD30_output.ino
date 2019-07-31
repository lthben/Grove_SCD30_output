//Connect sensor to I2C port of Grove shield
//See PDF design guidelines on how to install the sensor
//auto-calibration for first 7 days

#include "SCD30.h"

#if defined(ARDUINO_ARCH_AVR)
#pragma message("Defined architecture for ARDUINO_ARCH_AVR.")
#define SERIAL Serial
#elif defined(ARDUINO_ARCH_SAM)
#pragma message("Defined architecture for ARDUINO_ARCH_SAM.")
#define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_SAMD)
#pragma message("Defined architecture for ARDUINO_ARCH_SAMD.")
#define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_STM32F4)
#pragma message("Defined architecture for ARDUINO_ARCH_STM32F4.")
#define SERIAL SerialUSB
#else
#pragma message("Not found any architecture.")
#define SERIAL Serial
#endif

#include "rgb_lcd.h"
rgb_lcd lcd;

#include "Adafruit_NeoPixel.h"

#ifdef __AVR__
#include <avr/power.h>
#endif

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, 6, NEO_GRB + NEO_KHZ800);


//data is 0 to 19 corresponding to psi of 0 to 200
void colorWipe(uint8_t data, uint8_t wait)
{
  for (uint8_t i = 0; i <= data; i++)
  {
    if (i <= 4) strip.setPixelColor(i, strip.Color(0, 255, 0)); //green psi:0-50
    else if (i > 4 && i < 10) strip.setPixelColor(i, strip.Color(255, 255, 0)); //yellow psi:51-100
    else if (i >= 10 && i < 15) strip.setPixelColor(i, strip.Color(255, 128, 0)); //orange psi:101-150
    else strip.setPixelColor(i, strip.Color(255, 0, 0)); //red psi:>151

    strip.show();
    delay(wait);
  }
}

void setup()
{
  Wire.begin();
  SERIAL.begin(115200);
  SERIAL.println("SCD30 Raw Data");
  scd30.initialize();
  //Calibration for minimum 7 days,after this ,close auto self calibration operation.
  scd30.setAutoSelfCalibration(1);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255); //set RGB colour and brightness here
  lcd.setCursor(0, 0);

  lcd.clear();
  lcd.print("CO2 Sensor ready");
  delay(2000);

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  float result[3] = {0};

  if (scd30.isAvailable())
  {
    scd30.getCarbonDioxideConcentration(result);
    SERIAL.print("Carbon Dioxide Concentration is: ");
    SERIAL.print(result[0]);
    SERIAL.println(" ppm");
    SERIAL.println(" ");
    //    SERIAL.print("Temperature = ");
    //    SERIAL.print(result[1]);
    //    SERIAL.println(" ℃");
    //    SERIAL.println(" ");
    //    SERIAL.print("Humidity = ");
    //    SERIAL.print(result[2]);
    //    SERIAL.println(" %");
    //    SERIAL.println(" ");
    //    SERIAL.println(" ");
    //    SERIAL.println(" ");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO2 reading:");
  lcd.setCursor(0, 1);
  lcd.print(result[0]);
  lcd.print(" ppm");

  int data = constrain(result[0], 380, 450);//constrain ppm
  data = map(data, 380, 450, 0, 19);//convert 380-450ppm to 0-19 leds
  uint8_t wait = 100;

  strip.clear();
  strip.show();//turn off
//  delay(500);

  colorWipe(data, wait);

  delay(5000);
}
