/*
 * SonoffB1.h
 *
 *  Created on: 28.01.2018
 *      Author: a4711
 */

#ifndef SRC_SONOFFB1_H_
#define SRC_SONOFFB1_H_

class my92xx;

/* SONOFF B1 has two led driver of type "my9231",
 * each of them controlling three leds.
 * The first driver controlls warm and cold leds,
 * and the second one controlls red, green and blue.
 *
 * ch0 -> cold
 * ch1 -> warm
 * ch2 -> ----
 * ch3 -> green
 * ch4 -> red
 * ch5 -> blue
 */

class SonoffB1
{
  const static unsigned char MY92XX_DI_PIN = 12;  // MTDI  -> GPIO 12
  const static unsigned char MY92XX_DCKI_PIN = 14; // MTMS -> GPIO 14

  enum Channel
  {
    chCold = 0,
    chWarm = 1,
    chRed  = 4,
    chGreen = 3,
    chBlue = 5
  };


public:
  SonoffB1 ();
  virtual ~SonoffB1 ();

  void setup ();
  void updateChannel (unsigned char channel, unsigned int value);
  void controlLeds (const unsigned int* values, size_t length);
  void controlLeds (const char* message);
  void controlLeds (unsigned int cold, unsigned int warm, unsigned int red, unsigned int green, unsigned int blue);

private:
  my92xx* leds = nullptr;
};


#endif /* SRC_SONOFFB1_H_ */
