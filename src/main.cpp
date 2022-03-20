#include "main.h"
#include <Arduino.h>

void setup()
{
  Serial.begin(115200); //Begin Serial Communication

  //Set Pinmodes
  pinMode(PinDefs::PIN_RED, OUTPUT);
  pinMode(PinDefs::PIN_GREEN, OUTPUT);
  pinMode(PinDefs::PIN_BLUE, OUTPUT);
  pinMode(PinDefs::PIN_RLY1, OUTPUT);
  pinMode(PinDefs::PIN_ILED, OUTPUT);

  //Set Default Value for Relais
  digitalWrite(PinDefs::PIN_RLY1, RELAIS);
  digitalWrite(PinDefs::PIN_ILED, LOW);

  //Start Animation - Off to Full Red
  for (byte i = 0x00; i < 0xfe; i++)
  {
    analogWrite(PinDefs::PIN_RED, i);
    delay(EFFECTSPEED * 2);
  }
}

//Function to set appropriate Colors
void setColor(int cR, int cG, int cB)
{
  analogWrite(PinDefs::PIN_RED, cR);
  analogWrite(PinDefs::PIN_GREEN, cG);
  analogWrite(PinDefs::PIN_BLUE, cB);
  return;
}

//Checks if a Color is #FFFFFF and resets the colorStep value is so or the end if reached
void countContinuous()
{
  if (color[colorStep].red == 0xff && color[colorStep].green == 0xff && color[colorStep].blue == 0xff)
  {
    colorStep = 0;
  }
  setColor(color[colorStep].red, color[colorStep].green, color[colorStep].blue);
  colorStep++;
  if (colorStep >= animationLength)
  {
    colorStep = 0;
  }
  return;
}

//This function is similar to countContinous() but it doesn't reset colorStep but couts backwards instead
void countFwdBwd()
{
  if ((color[colorStep].red == 0xff && color[colorStep].green == 0xff && color[colorStep].blue == 0xff) || (colorStep >= animationLength - 1))
  {
    atEnd = true;
  }
  else if (colorStep <= 0)
  {
    atEnd = false;
  }
  setColor(color[colorStep].red, color[colorStep].green, color[colorStep].blue);
  if (!atEnd)
  {
    colorStep++;
  }
  else
  {
    colorStep--;
  }

  return;
}

//This function watis for serial data to read and returns the incomming byte !NO TIMEOUT!
int readSerialIfDataAvailable()
{
  while (Serial.available() < 1)
  {
    //WAIT IF DATA TO SLOW
  }
  return Serial.read();
}

//This function receives a up to 1542 byte long block of Serial data and stores it at *ptr
void getSerial(uint8_t *ptr)
{
  uint_least16_t firstByte = readSerialIfDataAvailable();
  firstByte = firstByte << 8;
  firstByte = firstByte + readSerialIfDataAvailable();
  readSerialIfDataAvailable();
  if (bitRead(firstByte, 0)) //Read if color[] should be updated
    for (size_t currentByte = 0; currentByte < 3 * animationLength; currentByte++)
    {
      *(ptr + currentByte) = readSerialIfDataAvailable();
    }
  if (bitRead(firstByte, 1)) // Read if EffectSpeed should be updated
    *(ptr + (3 * animationLength)) = readSerialIfDataAvailable();
  byte ctrlG = readSerialIfDataAvailable();
  byte ctrlB = readSerialIfDataAvailable();
  if (bitRead(firstByte, 8)) //Read if AnimBounce should be updated
    bitWrite(*(ptr + (3 * animationLength) + 1), 0, bitRead(ctrlG, 0));
  if (bitRead(firstByte, 9)) //Read if Table Relais should be updated
    bitWrite(*(ptr + (3 * animationLength) + 1), 1, bitRead(ctrlG, 1));
  if (bitRead(firstByte, 15)) //Read if Reset ColorStep should be updated
    bitWrite(*(ptr + (3 * animationLength) + 2), 0, bitRead(ctrlB, 0));

  //Serial.readBytes(color[0].red, 3 * (animationLength + 1));
  return;
}

void loop()
{
  digitalWrite(PinDefs::PIN_RLY1, RELAIS);

  if (Serial.available() > 0)
  {
    getSerial(&color[0].red);
    if (RESET_AT_TRANSMIT)
    {
      colorStep = 0;
    }
  }

  if (millis() > (lastMillis + (EFFECTSPEED * 2)))
  {
    lastMillis = millis();
    if (WRAPPING_BIT)
    {
      countFwdBwd();
    }
    else
    {
      countContinuous();
    }
  }
}
