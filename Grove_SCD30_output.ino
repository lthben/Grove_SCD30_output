//Connect sensor to I2C port of Grove shield
//RGB ring to D6
//LCD to I2C

//USER SETTINGS
bool IS_CALIBRATED = true;//set to true once calibrated. NEED TO DO ONCE ONLY 
int forcedCalibVal = 435;//forced calibration value in ppm

/*
 * Location: Chinatown Point main entrance
 * calibrated value -> measured value 
 * 445 -> 375
 * 435 -> 425
 */

int heightasl = 10;//height above sea level in metres
int ambientAirPres = 1010;//air pressure in mBar
int lowerCO2bound = 300; //to constrain the CO2 reading for meaningful pixel mapping
int upperCO2bound = 500; //to constrain the CO2 reading for meaningful pixel mapping
uint8_t wait = 100; //delay for the LED ring light sequence

#include "SparkFun_SCD30_Arduino_Library.h"
#include <Wire.h>

SCD30 airSensor;
long timeBegin;//time when the program starts

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
  Serial.begin(9600);

  airSensor.begin(); //This will cause readings to occur every two seconds

  //Change number of seconds between measurements: 2 to 1800 (30 minutes)
  //While calibrating for 2 min, should set interval to 2 sec
  airSensor.setMeasurementInterval(2);//interval of 5 screws up the reading

  //My desk is ~1600m above sealevel
  airSensor.setAltitudeCompensation(heightasl); //Set altitude of the sensor in m

  //Pressure in Boulder, CO is 24.65inHg or 834.74mBar
  airSensor.setAmbientPressure(ambientAirPres); //Current ambient pressure in mBar: 700 to 1200

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255); //set RGB colour and brightness here
  lcd.setCursor(0, 0);
  lcd.clear();
  if (!IS_CALIBRATED)
  {
    lcd.print("Initialising CO2 Sensor");
    Serial.println("Initialising CO2 Sensor");
  }
  else
  {
    lcd.print("Sensor ready");
    Serial.println("Sensor ready");
    delay(2000);
  }

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

  timeBegin = millis();
}

void loop()
{
  if (!IS_CALIBRATED) {
    if (millis() - timeBegin > 150000) //2.5 min
    {
      airSensor.setForcedRecalibrationFactor(forcedCalibVal);//global ppm is 415ppm
      IS_CALIBRATED = true;
    }
  }

  int CO2reading;
  float humidReading, tempReading;

  if (airSensor.dataAvailable())
  {
    CO2reading =  airSensor.getCO2();
    tempReading = airSensor.getTemperature();
    humidReading = airSensor.getHumidity();

    if (IS_CALIBRATED)
    {
      Serial.print("co2(ppm):");
      Serial.print(CO2reading);

      Serial.print(" temp(C):");
      Serial.print(tempReading, 1);

      Serial.print(" humidity(%):");
      Serial.print(humidReading, 1);

      Serial.println();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CO2 reading:");
      lcd.setCursor(0, 1);
      lcd.print(CO2reading);
      lcd.print(" ppm");

      int data = constrain(CO2reading, lowerCO2bound, upperCO2bound);//constrain ppm
      data = map(data, lowerCO2bound, upperCO2bound, 0, 19);//convert 380-450ppm to 0-19 leds

      strip.clear();
      strip.show();//turn off
      //  delay(500);

      colorWipe(data, wait);
    }
    else Serial.println("Initialising sensor ... ");
  }
  delay(2000);//min of 2 sec
}
