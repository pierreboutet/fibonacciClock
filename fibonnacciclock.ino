    // This project is protected under the Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
    // Attribute this work to  https://github.com/pchretien/fibo

    
    
    // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
    #include <Wire.h>
    #include "RTClib.h"
    
    // manage Leds with NeoPixel Librairie
    #include <Adafruit_NeoPixel.h>
    
    #define HOUR_PIN 6
    #define MINUTE_PIN 7
    #define BTN_PIN 8
    #define SET_PIN 10
    #define LED_PIN 4
    #define LED_COUNT 9
    #define DEBOUNCE_DELAY 10
    #define MAX_BUTTONS_INPUT 20
    #define TOTAL_PALETTES 10
    #define CLOCK_PIXELS 5
    #define MAX_MODES 1
    
    
    // Create an instance of the Adafruit_NeoPixel class called "leds".
    Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
    
    byte bits[CLOCK_PIXELS];
    
    uint32_t black = leds.Color(0,0,0);
    uint32_t colors[TOTAL_PALETTES][4] = 
    {
      {
        // #1 RGB
        leds.Color(255,255,255),    // off
        leds.Color(255,10,10),  // hours
        leds.Color(10,255,10),  // minutes
        leds.Color(10,10,255) // both;
      }, 
      {
        // #2 Mondrian
        leds.Color(255,255,255),    // off
        leds.Color(255,10,10),  // hours
        leds.Color(248,222,0),  // minutes
        leds.Color(10,10,255) // both;
      }, 
      {
        // #3 Basbrun
        leds.Color(255,255,255),    // off
        leds.Color(80,40,0),  // hours
        leds.Color(20,200,20),  // minutes
        leds.Color(255,100,10) // both;
      },
      {
        // #4 80's
        leds.Color(255,255,255),    // off
        leds.Color(245,100,201),  // hours
        leds.Color(114,247,54),  // minutes
        leds.Color(113,235,219) // both;
      }
      ,
      {
        // #5 Pastel
        leds.Color(255,255,255),    // off
        leds.Color(255,123,123),  // hours
        leds.Color(143,255,112),  // minutes
        leds.Color(120,120,255) // both;
      }
      ,
      {
        // #6 Modern
        leds.Color(255,255,255),    // off
        leds.Color(212,49,45),  // hours
        leds.Color(145,210,49),  // minutes
        leds.Color(141,95,224) // both;
      }
      ,
      {
        // #7 Cold
        leds.Color(255,255,255),    // off
        leds.Color(209,62,200),  // hours
        leds.Color(69,232,224),  // minutes
        leds.Color(80,70,202) // both;
      }
      ,
      {
        // #8 Warm
        leds.Color(255,255,255),    // off
        leds.Color(237,20,20),  // hours
        leds.Color(246,243,54),  // minutes
        leds.Color(255,126,21) // both;
      }
      ,
      {
        //#9 Earth
        leds.Color(255,255,255),    // off
        leds.Color(70,35,0),  // hours
        leds.Color(70,122,10),  // minutes
        leds.Color(200,182,0) // both;
      }
      ,
      {
        // #10 Dark
        leds.Color(255,255,255),    // off
        leds.Color(211,34,34),  // hours
        leds.Color(80,151,78),  // minutes
        leds.Color(16,24,149) // both;
      }
    };
    
    

    RTC_DS1307 RTC;
    
    int lastButtonValue[MAX_BUTTONS_INPUT];
    int currentButtonValue[MAX_BUTTONS_INPUT];

    int lightLevelIndex = 0;
    
    int ledsValues[5] = {1,1,2,3,5};
    
    byte oldHours = 0;
    byte oldMinutes = 0;
     
    int mode = 0;
    int palette = 0;
    
    void setup () {
        Serial.begin(57600);
        
        pinMode(HOUR_PIN, INPUT_PULLUP);
        pinMode(MINUTE_PIN, INPUT_PULLUP);
        pinMode(BTN_PIN, INPUT_PULLUP);
        pinMode(SET_PIN, INPUT_PULLUP);
        // define pull up (intern resistance is conneced to +5V) , 
        // so if pin is not connected to GND, it will be set to 1 by default
        //digitalWrite(HOUR_PIN, HIGH) ;
        //digitalWrite(MINUTE_PIN, HIGH) ;
        //digitalWrite(BTN_PIN, HIGH) ;
        //digitalWrite(SET_PIN, HIGH) ;
        
        // initialise RTC Clock module
        Wire.begin();
        RTC.begin();
        if (! RTC.isrunning()) {
          Serial.println("RTC is NOT running!");
          // following line sets the RTC to the date & time this sketch was compiled
          RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
        
        //initialise Leds
        leds.begin();
        leds.show();
    }
    
    void calcInstructionOne(int time, byte instr[]) {
      int remain[] = {0, 1, 2, 4, 7 };
      for (int i=4; i >= 0; i--) {
        if (time > remain[i] ) {
          time -= ledsValues[i];
          instr[i] = 1;
        } else {
          instr[i] = 0;
        }
        //instr[time%5] = 1;
      }
    }
    
    void calcInstructionTwo(int time, byte instr[]) {
      for( int i = 4; i >= 0; i--) {
         if(time >= ledsValues[i]) {
           time -= ledsValues[i];
           instr[i] = 1;
         } else {
           instr[i] = 0;
         }
      }
    }
    
    void dispTime(int hours, int minutes) {
      byte hoursInstr[CLOCK_PIXELS];
      byte minsInstr[CLOCK_PIXELS];
      
      if(hours >= 12) {
        //calcInstructionOne(hours % 12, hoursInstr);
        calcInstructionTwo(hours % 12, hoursInstr);
      } else {
        calcInstructionOne(hours, hoursInstr);
      }
      calcInstructionOne(minutes / 5, minsInstr);
      
      for(int i=0; i<CLOCK_PIXELS; i++)
        bits[i] = (hoursInstr[i] * 0x01) | (minsInstr[i] * 0x02);
        
      leds.begin();
      for(int i=0; i<CLOCK_PIXELS; i++)
      {
        setPixel(i, colors[palette][bits[i]]);
        //setPixel(i, colors[palette][0]);
      }
      leds.show();
    }
    
    void rainbow() {
    }
    void rainbow2() {
    }
    
    int hour = 1;
    int minute = 2;

    uint8_t lightLevels[6] = {255, 192, 128, 64, 32, 16 };
    
    void changeLightLevel() {
      lightLevelIndex = (lightLevelIndex + 1) % 6;
      leds.setBrightness(lightLevels[lightLevelIndex]);
    }

    void addFiveMin() {
      DateTime fixTime = RTC.now();
    
      DateTime newTime = DateTime(
          fixTime.year(), 
          fixTime.month(), 
          fixTime.day(), 
          fixTime.hour(), 
          ((fixTime.minute()-fixTime.minute()%5)+5)%60, 
          0);
          
      RTC.adjust( newTime );
    }
    
    void addOneHour() {
      DateTime fixTime = RTC.now();
    
      DateTime newTime = DateTime(
          fixTime.year(), 
          fixTime.month(), 
          fixTime.day(), 
          fixTime.hour() + 1, 
          fixTime.minute(), 
          0);
          
      RTC.adjust( newTime );
    }
     
    void loop () {
        DateTime now = RTC.now();
     
        printDate(now);
        
        // Read buttons
        int hour_button = debounce(HOUR_PIN);
        int minute_button = debounce(MINUTE_PIN);
        int button = debounce(BTN_PIN);
        int set_button = debounce(SET_PIN);
        
  /*
  Serial.print('d');
  Serial.print(hour_button, DEC);
  Serial.print(minute_button, DEC);
  Serial.print(button, DEC);
  Serial.print(set_button, DEC);
  Serial.println();*/
        
        delay(100);
        
        if(hour_button && hasChanged(HOUR_PIN)) {
            hour = (hour +1) % 5;
            addOneHour();
            Serial.print("hour change");
            Serial.println();
        }
        if(minute_button && hasChanged(MINUTE_PIN)) {
            //minute = (minute +1) % 5;
            addFiveMin();
        }
        if(button && hasChanged(BTN_PIN)) {
            changeLightLevel();
        }
        if( set_button && hasChanged(SET_PIN)) {
            themeChange();
        }
        resetButtonValues();
        //hour = (hour +1) % 5;
        //minute = (minute +1) % 5;

        switch (mode) {
          case 0 :
              dispTime(now.hour(), now.minute());
              break;
          case 1 :
              rainbow();
              break;
          case 2 :
              rainbow2();
              break;
        }
    }
    
    void themeChange()
    {
      palette = (palette +1) % TOTAL_PALETTES;
    }
    
    void modeChange()
    {
      mode = (mode +1) % MAX_MODES;
    }
    
    void setPixel(byte pixel, uint32_t color)
    {
      
      switch(pixel)
      {
        case 0:
          leds.setPixelColor(0, color);
          break;
        case 1:
          leds.setPixelColor(1, color);
          break;
        case 2:
          leds.setPixelColor(2, color);
          break;
        case 3:
          leds.setPixelColor(3, color);
          leds.setPixelColor(4, color);
          break;
        case 4:
          leds.setPixelColor(5, color);
          leds.setPixelColor(6, color);
          leds.setPixelColor(7, color);
          leds.setPixelColor(8, color);
          leds.setPixelColor(9, color);
          break;
      };
    }
      
    void printDate(DateTime now) {
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print(' ');
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.print(now.second(), DEC);
        Serial.println();
        Serial.println();
    }
    
void resetButtonValues()
{
  for(int i=0; i<MAX_BUTTONS_INPUT; i++)
    lastButtonValue[i] = currentButtonValue[i];
}
boolean hasChanged(int pin)
{
  return lastButtonValue[pin] != currentButtonValue[pin];
}
    
int debounce(int pin)
{
  int val = digitalRead(pin);
  currentButtonValue[pin] = val;
  if(val == 1) {
    return 0;
  } else {
    return 1;
  }
}
