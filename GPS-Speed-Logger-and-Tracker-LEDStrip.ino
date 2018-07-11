//GPS Library
#include <TinyGPS.h>

//SD Card Library
#include <SD.h>
//Enables serial connection to any pin
#include <SoftwareSerial.h>
//Enables communication to SPI devices
#include <SPI.h>

//LED Strip Libraries
#include <Adafruit_NeoPixel.h>
#define PIN      6 //use pin 6 to output data
#define N_LEDS 9  //number of LEDs (we have 10 on a strip but we only need 9)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);


float flat, flon;
unsigned long age;
  

TinyGPS gps;
File myFile;
String fileName = "trip-log.csv";

// The serial connection to the GPS device
//uses pins 2 and 3 to output data
//3 is input
//2 is output
SoftwareSerial ss(2, 3);

void setup()
{
  //setup strip
  strip.begin();
  strip.setBrightness(10);

  //***SD card board may be broken right now, cannot record rides***
  Serial.begin(9600);
  ss.begin(9600);
  delay(2000);
  
  if(!SD.begin(9)) {
    delay(500);
    Serial.println("sd card initialization failed!");
  }
}

void loop()
{
  displaySpeed(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  
  Serial.print("GPS SPEED: ");
  Serial.println(gps.f_speed_kmph());
  
  Serial.print("NUM SATELLITES: ");
  Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
  
  //test, works on its own
  //chase(strip.Color(255, 127, 0)); // Orange
  //chase(strip.Color(127, 255, 0)); // Yellow
  //chase(strip.Color(0, 255, 0));  // Green
  
  //get current position
  gps.f_get_position(&flat, &flon, &age);
  //delay for 1 sec
  smartDelay(1000);
  
}

static void chase(uint32_t c) {
  Serial.println("CHASE");
  for(uint16_t i = 0; i < strip.numPixels()+10; i++) {
      strip.setPixelColor(i  , c); // Draw new pixel
      strip.setPixelColor(i-10, 0); // Erase pixel a few steps back
      strip.show();
      delay(150);
  }
}


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  Serial.println("SMART DELAY");
  
  unsigned long start = millis();
  do 
  {
    //Serial.println("SMART DELAY: do");
    while (ss.available())
      Serial.println("SMART DELAY: while");
      gps.encode(ss.read());
  } while (millis() - start < ms);

  if(ss.available()){
    Serial.println("SS AVAILABLE");
  }
  else
  Serial.println("SS NOT AVAILABLE");
}

//error status
static void error()
{
  strip.setPixelColor(4, 255, 0, 0);
  strip.setPixelColor(5, 255, 0, 0);
  strip.show();
}

static void displaySpeed(float val, bool valid, int len, int prec)
{
  Serial.println("DISPLAY SPEED");
  String mySpeed = String(val,prec);  

  //ready the SD card for writing
  //myFile = SD.open("log.csv", FILE_WRITE);
  
  if(!valid)
  {
    while (len-- > 1){
      error(); //show error lights
    }
  }
  else
  {
    //display the speed in terms of lights
    setLights(val);
    

    if(mySpeed == "-1.00")
      error();
      return; 

//    //SD Card
//    if(myFile) {
//      myFile.print(print_date(gps));
//      myFile.print(",");
//      myFile.print(val, prec);  
//      myFile.print(",");
//      myFile.print(flat,6);
//      myFile.print(",");
//      myFile.println(flon,6);
//      myFile.close();
//      
//      Serial.println("SD WRITE");
//
//    }
  }
  smartDelay(0);
}

static uint32_t getColour(float val){
  int tens = int(val/10);
  uint32_t colour;

  //range of values from 0 - 159kmph
  switch(tens){
  case 0:
    //0-9kmph
    colour = strip.Color(255, 0, 0); // red
    break;
  case 1:
    //10-19kmph
    colour = strip.Color(255, 127, 0); // orange
    break;
  case 2:
    //20-29kmph
    colour = strip.Color(127, 255, 0); // yellow
    break;
  case 3:
    //30-39kmph
    colour = strip.Color(0, 255, 0); // green
    break;
  case 4:
    //40-49kmph
    colour = strip.Color(0, 255, 127); // green-blue
    break;
  case 5:
    //50-59kmph
    colour = strip.Color(0, 0, 255); // blue
    break;
  case 6:
    //60-69kmph
    colour = strip.Color(127, 0, 255); // violet
    break;
  case 7:
    //70-79kmph
    colour = strip.Color(255, 0, 255); // purple
    break;
  case 8:
    //80-89kmph
    colour = colour = strip.Color(255, 0, 127); // pink
    break;
  case 9:
    //90-99
    colour = strip.Color(255, 255, 255); // white
    break;
  case 10:
    //100-109kmph
    colour = strip.Color(255, 0, 0); // red
    break;
  case 11:
    //110-119kmph
    colour = strip.Color(255, 127, 0); // orange
    break;
  case 12:
    //120-129kmph
    colour = strip.Color(127, 255, 0); // yellow
    break;
  case 13:
    //130-139kmph
    colour = strip.Color(0, 255, 0); // green
    break;
  case 14:
    //140-149kmph
    colour = strip.Color(0, 255, 127); // green-blue
    break;
  case 15:
    //150-159kmph
    colour = strip.Color(0, 0, 255); // blue
    break;

  default:
    colour = strip.Color(0, 0, 0); //nothing
  }

  return(colour);
}

static int getNumPixels(float val){
  int ones = val - (int(val/10)*10);
  return(ones);
}

static void setLights(float val){
  uint32_t col = getColour(val);
  int pix = getNumPixels(val);

  switch(pix) {
    case 0: //x0, no pixels on, clear all
      break;
    case 1: //x1, 1 pixel on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 2: //x2, 2 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 3: //x3, 3 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 4: //x4, 4 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 5: //x5, 5 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 6: //x6, 6 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 7: //x7, 7 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 8: //x8, 8 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;
    case 9: //x9, 9 pixels on
      strip.setPixelColor(pix-1, col); //turn on pixel at pix-1, colour col
      break;

    default:
      break;
  }
  
  strip.show();
}

static String print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d_%02d_%02d",
        hour, minute, second);
    return(sz);
  }
  smartDelay(0);
}

