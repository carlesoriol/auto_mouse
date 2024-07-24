/*
    by Carles Oriol 2019-2024
*/

#include <Mouse.h>
#include <EEPROM.h>
#include <Keyboard.h>


#define RGBLED
#define EXTRAKEY

#ifdef ENABLE_CLASSIC // for BT support
  
  //#define BLE_MOUSE
  //#include <MouseBLE.h>

  #define BT_MOUSE
  #include <MouseBT.h>

#endif



#ifdef RGBLED
  #include <NeoPixelBus.h>
  const int pinWS2812 = 23;
                           
  NeoPixelBus<NeoBgrFeature, NeoWs2812xMethod> rgbled(1, pinWS2812); 
  
#endif

#ifdef EXTRAKEY
  const int pinUserKey = 24;
#endif

unsigned long animationTimes[3] = {15000L, 60000L, 300000L}; // 15s, 1m, 5m
unsigned long animationTime = 15000;
int animationMode = 0;
const int animationModes = 7;


#define EVERYMS(s, F)       \
  {                         \
    static long tv = -s;    \
    if (millis() - tv >= s) \
    {                       \
      tv = millis();        \
      F;                    \
    }                       \
  }

#ifdef RGBLED

//  BRG

RgbColor LedColors[] = {
  RgbColor(  0, 0, 0),          // 0 none

  RgbColor(255, 0, 0),        // 1 mouse
  RgbColor(255, 255 , 0),      // 2
  RgbColor(64, 64, 64),        // 3
  RgbColor(0, 255, 0),      // 4

  RgbColor(0, 0, 255),      // 5 keyboard
  RgbColor(0, 64, 64),      // 6
};

void showled()
{
    rgbled.ClearTo(0);
    rgbled.SetPixelColor(0, LedColors[animationMode]);
    rgbled.Show();
 
}

#endif

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Mouse.begin();
  Keyboard.begin();
  
  #ifdef BLE_MOUSE
  MouseBLE.begin("Charly Dummy Mouse BLE");
  #endif

  #ifdef BT_MOUSE
  MouseBT.begin("Charly Dummy Mouse ");
  #endif

  EEPROM.begin(512);
  animationMode = EEPROM.read(0) % animationModes; 
  

  #ifdef RGBLED
    rgbled.Begin();
    showled();
  #endif

  #ifdef EXTRAKEY
    pinMode(pinUserKey, INPUT_PULLUP);
    int i = EEPROM.read(4) % 3; 
    setAnimTime( i);
  #endif
}

void move(int x, int y)
{
  Mouse.move(x, y, 0);
  #ifdef BLE_MOUSE
    MouseBLE.move(x, y, 0);
  #endif  
  #ifdef BT_MOUSE
    MouseBT.move(x, y, 0);
  #endif
  
}

int signum(int value) {
    return (0 < value) - (value < 0);
}

void animate()
{
  switch( animationMode)
  {
    // res
    case 0:
    {      
      break;
    }

    case 1: // cercles
    {
      const float r = 75;
      float ox = 0.0;
      float oy = 0.0;
      for (float a = 0; a < 2.0 * 3.14159; a += 0.1)
      {
        float ax = r * cos(a);
        float ay = r * sin(a);
        move(ax - ox, ay - oy);
        ox = ax;
        oy = ay;
        delay(10);
      }
      break;
    }

    // linies diagonals
    case 2:
    case 3: 
    {
      static int direction = 1;
      const int stepSize = 1;

      for (int i = 0; i < (animationMode == 2 ? 50 : 2); i++)
      {
        move(stepSize * direction, stepSize * direction);
        delay(10);
      }
      direction *= -1;
      break;
    }

    // moviment aleatori
    case 4:
    {
      int nx = random(-500, 500);
      int ny = random(-500, 500);
      for (int i = 0; i < max(abs(nx), abs(ny)); i+=4)
      {
        move(((i < abs(nx)) ? signum(nx) : 0), ((i < abs(ny)) ? signum(ny) : 0));
        delay(1);
      }
      break;
    }

    // keyboard modes
    // puja i baixa el volum
    case 5:
    {
      Keyboard.consumerPress(KEY_VOLUME_INCREMENT);
      delay(100);
      Keyboard.consumerRelease();
      delay(10);
      Keyboard.consumerPress(KEY_VOLUME_DECREMENT);
      delay(100);
      Keyboard.consumerRelease();
      break;
    }

    // escriu un espai
    case 6:
    {
      Keyboard.print(" ");      
      break;
    }
  }
  
}

#ifdef EXTRAKEY
int getAnimTime()
{
  if( animationTime == animationTimes[2])
    return 2;
  
  if( animationTime == animationTimes[1])
    return 1;
  
  return 0;  
}

void setAnimTime(int t)
{
  animationTime = animationTimes[t];
  
  for( int i = 0; i < t + 1; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
#endif

void loop()
{
  EVERYMS(animationTime, animate());

  if (BOOTSEL)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    animationMode++;
    animationMode %= animationModes;
    EEPROM.write(0, animationMode);
    EEPROM.commit();
    animate();
    while (BOOTSEL)
    {
      delay(1);
    }
    digitalWrite(LED_BUILTIN, LOW);  

    #ifdef RGBLED
      showled();
    #endif
  }

#ifdef EXTRAKEY
  if( digitalRead(pinUserKey) == LOW)
  {
    int i = getAnimTime();
    i++;
    i %= 3;
    setAnimTime(i);
    
    EEPROM.write(4, i);
    EEPROM.commit();

    while( digitalRead(pinUserKey) == LOW) delay(10);
  }
  #endif

}

      

      