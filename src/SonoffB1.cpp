/*
 * SonoffB1.cpp
 *
 *  Created on: 28.01.2018
 *      Author: a4711
 */

#include <my92xx.h>
#include "SonoffB1.h"

SonoffB1::SonoffB1 ()
{
  // TODO Auto-generated constructor stub

}

SonoffB1::~SonoffB1 ()
{
  // TODO Auto-generated destructor stub
}

void SonoffB1::setup ()
{
  leds = new my92xx (MY92XX_MODEL_MY9231, 2, MY92XX_DI_PIN, MY92XX_DCKI_PIN,
                         MY92XX_COMMAND_DEFAULT);
  leds->setState (true);
}

void SonoffB1::updateChannel (unsigned char channel, unsigned int value)
{
  leds->setChannel (channel, value);
  leds->update ();
}

void SonoffB1::controlLeds (const char* message)
{
  Serial.print ("message: ");
  Serial.println (message);
  unsigned int values[] =
  { 0, 0, 0, 0, 0 };
  char buffer[5];
  size_t tmpIdx = 0;
  size_t valIdx = 0;
  for (; 0 != message; message++)
  {
    if (',' == *message || '\0' == *message)
    {
      buffer[tmpIdx] = 0;
      tmpIdx = 0;
      if (valIdx < (sizeof(values) / sizeof(values[0])))
      {
        values[valIdx++] = ::atoi (buffer);
      }
      else
      {
        break; // no more space in values array
      }
      if ('\0' == *message)
      {
        break;
      }
    }
    else if (tmpIdx < (sizeof(buffer) / sizeof(buffer[0])))
    {
      buffer[tmpIdx++] = *message;
    }
  }
  controlLeds (values, sizeof(values) / sizeof(values[0]));
}

void SonoffB1::controlLeds (const unsigned int* values, size_t length)
{
  // (c, w, r, g b)  // cold, warm, red, green, blue
  unsigned char channelMap[] =
  { chCold, chWarm, chRed, chGreen, chBlue };
  length = min(length, sizeof(channelMap) / sizeof(channelMap[0]));
  for (size_t i = 0; i < length; i++)
  {
    Serial.print ("idx: ");
    Serial.print (i);
    Serial.print (" val: ");
    Serial.println (values[i]);
    if (i >= sizeof(channelMap) / sizeof(channelMap[0]))
      break;

    leds->setChannel (channelMap[i], values[i]);
  }
  leds->update ();
}

void SonoffB1::controlLeds (unsigned int cold, unsigned int warm, unsigned int red, unsigned int green, unsigned int blue)
{
  leds->setChannel (chCold, cold);
  leds->setChannel (chWarm, warm);
  leds->setChannel (chRed, red);
  leds->setChannel (chGreen, green);
  leds->setChannel (chBlue, blue);
  leds->update ();
}


