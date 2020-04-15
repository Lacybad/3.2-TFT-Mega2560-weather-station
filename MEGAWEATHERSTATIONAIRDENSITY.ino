/*
  Download all Libraries and include to your arduino libraries folder to avoid the compiling error..
  <display_model, 38,39,40,41> is for arduino mega,, check PDF file of UTFT library to connect on other microcontroller(arduino)
*/

#include <TFT_HX8357.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <DS3231.h>
#include <math.h>

TFT_HX8357 tft = TFT_HX8357();

//Don't uncomment
//TFT_HX8357   tft(ILI9481,38,39,40,41);// Select your display modl <display_model,38,39,40,41>


DS3231  rtc(SDA, SCL);

#define BMP085_ADDRESS                (0x77)
Adafruit_BMP085 bmp;

#define ONE_WIRE_BUS 10
#define DHTPIN 11

//#define DHTTYPE DHT11
#define DHTTYPE DHT21
//#define DHTTYPE DHT22

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define GREY    0x2108

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);

//////////////////////////////////////////////////

  const float c0 = 6.1078;           //constants for satVapourPress
  const float c1 = 7.5;
  const float c2 = 237.3;
  const float RdryAir = 286.9;       // gas constant for dry air J/kg K
  const float RwaterVapour = 461.5;  // gas constant for water vapour J/kg K
  const float dA1 = 44.3308;         // constants for densityAlt
  const float dA2 = 42.2665;
  const float dA3 = 0.234969;
  const float kmToFt = 3280.0;        // converts km to feet
  const float cToK = 273.15;          // converts *C to *K
  const float mbToPasc = 100;         // converts millibars to pascals

  const int Rx = 16; //the micro can't use pin 2 for Rx
  const int Tx = 5;

  float satVapPress;        // saturation vapour pressure mb
  float parPressWatVap;     // partial pressure of water vapour mb
  float parPressDryAir;     // partial pressure of dry air mb
  float airDensity;         // air density kg/m3
  float densityAlt;         // density altitude in feet
  float tempVar1;           // temporary variables for long equations
  float tempVar2;
  float temperature;        // sensor temperature value
  //float rh;                 // sensor relative humidity value
  float pressure;           // sensor pressure value

int tempDA;

/****SO THAT END POINT XBEE 2 BYTE DATA CAN BE SENT AND REASSEMBLED AT COORDINATOR XBEE****/
  int LSBtempDA;
  int MSBtempDA;
  int LSBpressure;
  int MSBpressure;
  int divisor = 256;
  int rhInt;
  int temperatureInt;
  int pressureInt;

////////////////////////////////////////////////////////

  int LDRpin = 5;
  char lightString [4];
  String str;

  int lightIntensity = 0;

  uint32_t runTime = -99999;       // time for next update
  int reading = 0; // Value to be displayed
  int d = 0; // Variable used for the sinewave test waveform
  boolean alert = 0;
  int8_t ramp = 1;
  int tesmod = 0;

  float tempC = 0;

  char TempCelciusFahrenheit[6];

  float tempF = 0;


  float ah; //absolute humidity = water vapor density in g/m*3
  float rh; // relative humidity in %
  float t; //temperature in celsius
  float td; //dew point in celsius
  float tf; //temperature in fahrenheit
  float tdf; // dew point temperature in fahrenheit

//Formula to calculate Dew point
  double dewPoint(double celsius, double humidity) {
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);
  double VP = pow(10, RHS - 3) * humidity;
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}


//Formula to calculate Dew point fast

double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}


//Raindrop bitmap
static const unsigned char PROGMEM picture[] =
{ B0001000,
  B0001000,
  B0001000,
  B0010100,
  B0100010,
  B0100010,
  B0011100
};



void setup() {
  // Start the I2C interface
  Wire.begin();
  // Start the serial interface
  Serial.begin(115200);
  Serial.println("Weather Station test!");
  tft.init();
  tft.fillScreen( BLACK);
  dht.begin();
  bmp.begin();
  rtc.begin();
  randomSeed(analogRead(7));
  tft.setRotation(1); // Set Rotation at 0 degress (default) Don't change!

  // Uncommente to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to MONDAY, TUESDAY etc.
  //rtc.setTime(22, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(19, 1, 2018);   // Set the date to January 1st, 2014




  tft.fillScreen(BLACK); //Set Background Color with BLACK
  tft.setCursor (0, 5);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Temperature ");

  tft.setCursor (0, 35);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Fahrenheit ");

  tft.setCursor (0, 65);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Humidity");

  tft.setCursor (0, 95);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Heat Index F");

  tft.setCursor (0, 125);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Heat Index C");

  tft.setCursor (0, 185);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Dew Point");

  tft.setCursor (0, 155);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Light");

  tft.setCursor (200, 300);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("KELVIN");

  tft.setCursor (0, 300);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Ds temp");

  tft.setCursor (0, 215);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Dew p fast");

  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(230, 5);
  tft.println("TIME");

  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(230, 35);
  tft.println("DATE");

  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(205, 245);
  tft.println("Hpa");

  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 245);
  tft.println("Pressure");

  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(2);
  tft.setCursor(230, 65);
  tft.println("Altitude");

  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(1);
  tft.setCursor(425, 68);
  tft.println("meters");

  tft.setCursor (0, 272);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Prs");

  tft.setCursor (140, 272);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("mmHg");

  tft.setCursor (230, 95);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("Absolute Humidity");

  tft.setCursor (350, 125);
  tft.setTextSize (2);
  tft.setTextColor (WHITE, BLACK);
  tft.print ("g/m*3");

  /*tft.setCursor (230,150);
    tft.setTextSize (2);
    tft.setTextColor (CYAN,BLACK);
    tft.print ("Dew point fahrenheit");*/

  tft.drawRect (140, 2, 73, 20 , MAGENTA);
  tft.drawRect (125, 32, 65, 20 , BLUE);
  tft.drawRect (115, 62, 67, 20 , YELLOW);
  tft.drawRect (145, 92, 67, 20 , BLUE);
  tft.drawRect (150, 122, 40, 20 , MAGENTA);
  tft.drawRect (80, 152, 40, 20 , CYAN);
  tft.drawRect (122, 182, 80, 20 , GREEN);
  tft.drawRect (122, 212, 80, 20 , GREEN);
  tft.drawRect (105, 242, 95, 20 , BLUE);
  tft.drawRect (45, 269, 80, 20 , BLUE);
  tft.drawRect (85, 297, 100, 21 , RED);
  tft.drawRect (280, 297, 78, 21 , RED);
  tft.drawRect (340, 62, 79, 21 , BLUE);

}



void loop() {

  float tempC = 0;
  float tempF = 0;
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);
  tempF = sensors.toFahrenheit(tempC);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  rh = dht.readHumidity();
  t = dht.readTemperature();
  tf = t * 9 / 5 + 32;

  lightIntensity = analogRead(LDRpin);

  lightIntensity = map(lightIntensity, 1025, 20, 0, 98);

  str = String(lightIntensity);//+"%"
  str.toCharArray(lightString, 5);
  //rh = dht.readHumidity();

  satVapPress = c0 * pow(10, (c1 * temperature) / (c2 + temperature));   //calculate water vapour saturation pressure
  parPressWatVap = satVapPress  * rh / 100;                              //calculate water vapour partial pressure
  parPressDryAir = pressure - parPressWatVap;                            //calculate dry air partial pressure

  tempVar1 = parPressDryAir * mbToPasc / (RdryAir * (temperature + cToK));          //first term of airDensity
  tempVar2 =  parPressWatVap * mbToPasc / (RwaterVapour * (temperature + cToK));    //second term of airDensity
  airDensity = tempVar1 + tempVar2;                                                 //calculate air density

  int densityAlt = round((dA1 - dA2 * pow(airDensity, dA3)) * kmToFt);   //calculate density altitude in feet

  ////////////////////////////////////////////////////////
  temperature = dht.readTemperature();
  //temperature = bmp.readTemperature
  pressure = bmp.readPressure();

  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setCursor(250, 200);
  tft.print("Air Density");

  tft.setTextColor(RED, BLACK);
  tft.setTextSize(2);
  tft.setCursor(250, 225);
  tft.print(airDensity * 10);

  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setCursor(250, 255);
  tft.print("Dry air pressure");

  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(2);
  tft.setCursor(330, 275);
  tft.print(parPressDryAir / 100);

  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(2);
  tft.setCursor(430, 275);
  tft.print("hpa");

  tft.setTextColor(RED, BLACK);
  tft.setTextSize(2);
  tft.setCursor(370, 225);
  tft.print("g/m3");


  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setCursor(250, 125);
  tft.print(ah);

  /*tft.setTextColor(CYAN,BLACK);
    tft.setTextSize(2);
    tft.setCursor(250, 175);
    tft.print(tdf);*/

  ah = (6.112 * pow(2.71828, ((17.67 * t) / (243.5 + t))) * rh * 2.1674) / (273.15 + t);
  //tdf = (243.5*(log(rh/100)+((17.67*t)/(243.5+t)))/(17.67-log(rh/100)-((17.67*t)/(243.5+t))))*9/5+32;

  tft.drawBitmap(205, 64,  picture, 8, 8, 50);
  tft.drawBitmap(210, 70,  picture, 8, 8, 50);
  tft.drawBitmap(200, 70,  picture, 8, 8, 50);
  tft.drawBitmap(215, 185,  picture, 8, 8, 50);
  tft.drawBitmap(210, 190,  picture, 8, 8, 50);
  tft.drawBitmap(220, 190,  picture, 8, 8, 50);
  tft.drawBitmap(215, 215,  picture, 8, 8, 50);
  tft.drawBitmap(210, 220,  picture, 8, 8, 50);
  tft.drawBitmap(220, 220,  picture, 8, 8, 50);





  tft.setCursor(95, 300);  // Temperature from DS18B20
  tft.setTextSize (2);
  tft.setTextColor ( RED , BLACK);
  tft.print (tempC);
  tesmod = 1;

  tft.setCursor(285, 300);  // Temperature in Kelvin
  tft.setTextSize (2);
  tft.setTextColor ( RED , BLACK);
  tft.print (tempC + 273.15);
  tesmod = 1;

  tft.setCursor (160, 95); // Heat index in Fahrenheit
  tft.setTextSize (2);
  tft.setTextColor (BLUE, BLACK);
  tft.print (hif, 1);
  tesmod = 1;

  tft.setCursor (160, 125); // Heat index in celsius
  tft.setTextSize (2);
  tft.setTextColor (MAGENTA, BLACK);
  tft.print (hic, 0);
  tesmod = 1;

  tft.setCursor (137, 185); // Print the dew point
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print (dewPoint(t, h ));
  tesmod = 1;

  tft.setCursor (137, 215); // Print the dew point fast
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print (dewPointFast(t , h));
  tesmod = 1;

  tft.setCursor (184, 215);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("C");

  tft.setCursor (184, 185);
  tft.setTextSize (2);
  tft.setTextColor (GREEN, BLACK);
  tft.print ("C");


  tft.setCursor (140, 35); // Heat index in fahrenheit
  tft.setTextSize (2);
  tft.setTextColor (BLUE, BLACK);
  tft.print (f, 0);

  tft.setCursor (120, 65); // Print the Humidity
  tft.setTextSize (2);
  tft.setTextColor (YELLOW, BLACK);
  tft.print (h, 1); tft.print ('%');
  tesmod = 1;

  tft.setCursor (90, 155); // Print the light intensity in percentage
  tft.setTextSize(2);
  tft.setTextColor( CYAN, BLACK);
  tft.print(lightString);
  tesmod = 1;

  tft.setCursor (150, 5); // Print temperature from DHT sensor
  tft.setTextSize (2);
  tft.setTextColor (MAGENTA, BLACK);
  tft.print (t, 1); tft.print ('C');
  tesmod = 1;

  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(2);
  tft.setCursor(290, 5);
  tft.print(rtc.getTimeStr(FORMAT_SHORT)); // FORMAT_LONG for displaying seconds

  tft.setTextColor(YELLOW, BLACK); //Date display
  tft.setTextSize(2);
  tft.setCursor(290, 35);
  tft.print(rtc.getDateStr(FORMAT_SHORT));

  tft.setTextColor(CYAN, BLACK); // Display day of week
  tft.setTextSize(2);
  tft.setCursor(360, 5);
  tft.print(rtc.getDOWStr());

  tft.setTextColor(BLUE, BLACK); // Reading pressure
  tft.setTextSize(2);
  tft.setCursor(110, 245);
  tft.print(bmp.readPressure() / 98.6);

  tft.setTextColor(BLUE, BLACK); // Displaying Altitude
  tft.setTextSize(2);
  tft.setCursor(345, 65);
  tft.print(bmp.readAltitude());

  tft.setTextColor(BLUE, BLACK); // Displaying pressure in mmHg
  tft.setTextSize(2);
  tft.setCursor(50, 272);
  tft.print((bmp.readPressure() * 0.007501) + 12.13);





  /*if (millis() - runTime >= 1000) { // Execute every 500ms
    runTime = millis();*/

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  Serial.print("Fahrenheight: ");
  Serial.print(f);
  Serial.println(" *F\t");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %\t");
  Serial.print("Light: ");
  Serial.print(lightString);
  Serial.println(" % ");
  Serial.print("DS Temp: ");
  Serial.print(tempC);
  Serial.println(" *C ");
  Serial.print("Heat index F: ");
  Serial.print(hif);
  Serial.println(" *F ");
  Serial.print("Heat index C: ");
  Serial.print(hic);
  Serial.println(" *C ");
  Serial.print("Dew Point: ");
  Serial.print(dewPoint(t, h ));
  Serial.println(" Celsius ");
  Serial.print("Dew Point Fast: ");
  Serial.print(dewPointFast(t, h ));
  Serial.println(" Celsius ");
  Serial.print(rtc.getDateStr(FORMAT_SHORT));
  Serial.println("  Date");
  Serial.print(rtc.getTimeStr(FORMAT_SHORT));
  Serial.println("  Time");
  Serial.print(rtc.getDOWStr());
  Serial.println("  Day");
  Serial.println("Pressure");
  Serial.print(bmp.readPressure() / 98.6);
  Serial.println("  Hpa");
  Serial.print("Pressure : ");
  Serial.print((bmp.readPressure() * 0.007501) + 12.13);
  Serial.println(" mmHg ");
  Serial.print("Altitude: ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters ");
  Serial.println("Absolute humidity:");
  Serial.print(ah);
  Serial.println("  g/m3");
  Serial.println("Air Density:");
  Serial.print(airDensity * 10);
  Serial.println("  g/m3");
  Serial.println();


  delay(1000);
}


