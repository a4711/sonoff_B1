/*
 * Sunrise.h
 *
 *  Created on: 13.01.2018
 *      Author: a4711
 */

#ifndef SRC_SUNRISE_H_
#define SRC_SUNRISE_H_
#include <Arduino.h>
#include <functional>
#include <limits>
#include "myiot_timer_system.h"

class Sunrise : public MyIOT::ITimer
{
public:
  Sunrise ();
  virtual  ~Sunrise ();

  void start(uint32_t aDurationInSeconds)
  {
    durationInSeconds = aDurationInSeconds;
    startTime = millis();
  }

  void setup(std::function<void(uint16_t)> xValueChange)
  {
    onValueChange = xValueChange;
  }

  void reset()
  {
    durationInSeconds = 0;
    startTime = 0;
  }

  bool isRunning() const
  {
    return 0 != durationInSeconds;
  }

  bool isDone() const
  {
    return deltaSeconds() >= durationInSeconds;
  }

  void expire() override
  {
    if (isRunning())
    {
      if (isDone())
      {
        if (onValueChange) onValueChange(std::numeric_limits<uint16_t>::max());
        reset();
      }
      else
      {
        uint16_t currentValue = getCurrentValue();

        if (currentValue != lastPublishedValue)
        {
          if (onValueChange) onValueChange(currentValue);
          lastPublishedValue = currentValue;
        }
      }
    }
  }

  void destroy() override {}

private:

  unsigned long deltaSeconds() const
  {
    return (millis() - startTime) / 1000;
  }

  uint16_t getCurrentValue() const
  {
    float temp = getLinearFactor();
    temp = temp * temp; // square function
    return uint16_t(  temp * std::numeric_limits<uint16_t>::max());
  }

  float getLinearFactor() const
  {
    if (0 == durationInSeconds) return 0;
    return  (float)(millis() - startTime) / (float)(durationInSeconds * 1000);
  }

  std::function<void(uint16_t)> onValueChange;
  uint32_t durationInSeconds;
  unsigned long startTime;
  uint8_t lastPublishedValue;
};

#endif /* SRC_SUNRISE_H_ */
