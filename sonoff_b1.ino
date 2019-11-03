
#include <my92xx.h>
#include <string.h>
#include "src/myiot_timer_system.h"
#include "src/myiot_DeviceConfig.h"
#include "src/myiot_webServer.h"
#include "src/myiot_ota.h"
#include "src/myiot_Mqtt.h"
#include "src/Sunrise.h"
#include "src/SonoffB1.h"


MyIOT::TimerSystem tsystem;
MyIOT::Mqtt mqtt;
MyIOT::DeviceConfig config;
MyIOT::OTA ota;
MyIOT::WebServer webServer;

Sunrise sunrise;
SonoffB1 b1;





class ColorConfig
{
public:
  ColorConfig():ledColors{"200,200,0,0,0"}, enabled(false){}
  const char* getLedColors() const{return ledColors;}
  void setLedColors(const char* name) { strncpy(ledColors, name, sizeof(ledColors));    ledColors[sizeof(ledColors)-1] = 0; }

  bool getEnabled() const {return enabled;}
  void setEnabled(bool enable){enabled = enable;}

  void setup()
  {
     fsReadConfig();
  }

  void save(){fsSaveConfig();}
private:
  static constexpr const char * CONFIG_FILE = "/color_config.json";
  char ledColors[40];
  bool enabled;

  void fsReadConfig()
   {
     if (SPIFFS.begin())
     {
       if (SPIFFS.exists(CONFIG_FILE))
       {
         File configFile = SPIFFS.open(CONFIG_FILE, "r");
         if (configFile)
         {
           size_t size = configFile.size();
           std::unique_ptr<char[]> buffer(new char[size]); // dynamic memory !!!
           configFile.readBytes(buffer.get(), size);
           configFile.close();

           DynamicJsonBuffer jsonBuffer;
           JsonObject& json = jsonBuffer.parseObject(buffer.get());
           if (json.success())
           {
             auto jsonLedColors = json["ledColors"];
             if (jsonLedColors.success())
             {
               strncpy(ledColors, jsonLedColors, sizeof(ledColors));
             }

             auto jsonEnabled = json["enabled"];
             if (jsonEnabled.success())
             {
        	 enabled = 0 == ::strcmp("1", jsonEnabled);
             }

           } else error("failed to parse config file data");
         }
         else error("failed to open config file");
       }
       else info("no config file");
     }
     else error("SPIFFS.begin() failed");
   }
  void fsSaveConfig()
  {
    info("fsSaveConfig()");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    info("device_name", ledColors);

    json["ledColors"] = ledColors;
    json["enabled"] = enabled ? "1" : "0";
    File configFile = SPIFFS.open(CONFIG_FILE, "w");
    if (configFile)
    {
      json.printTo(configFile);
      configFile.close();
    }
    else error ("failed to save config file");
  }
  void error(const char* msg)
  {
    Serial.print("ColorConfig error: ");
    Serial.println(msg);
  }
  void info(const char* msg)
  {
    Serial.print("ColorConfig info: ");
    Serial.println(msg);
  }
  void info(const char* msg1, const char* msg2)
  {
    Serial.print("DeviceConfig info: ");
    Serial.print(msg1);
    Serial.print(" ");
    Serial.println(msg2);
  }
} colorConfig;


void setup() {
  Serial.begin(115200);

  config.setup();
  colorConfig.setup();

  ota.setup(config.getDeviceName());
  mqtt.setup(config.getDeviceName(), config.getMqttServer());
  webServer.setup(config);

  b1.setup();

#define SUNRISE
#if defined (SUNRISE)
  sunrise.setup([](uint16_t value){

    uint8_t high = value>>8;
    uint8_t low = 0xff & value;

    uint8_t w = high;
    uint8_t c = 0;

    uint8_t r = 255;
    if ( high < 0xF)
    {
       r =  (high<<4) | (low>>4);
    }

    uint8_t g = 0;
    uint8_t b = 0;

    unsigned int values[] = {c,w,r,g,b};
    b1.controlLeds(values, sizeof(values)/sizeof(values[0]) );
  });
#endif

  tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0, 10e6));
  tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0,10e6));
  tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0, 100e6));
  tsystem.add(&sunrise, MyIOT::TimerSystem::TimeSpec(0, 100e6));


  //mqtt.setOnConnected( [] () {mqtt.publish("system", "MQTT connected");});

#if defined(TESTCHANNELS)
  mqtt.subscribe("ch0", [](const char* message){ b1.updateChannel(0, ::atoi(message)); });
  mqtt.subscribe("ch1", [](const char* message){ b1.updateChannel(1, ::atoi(message)); });
  mqtt.subscribe("ch2", [](const char* message){ b1.updateChannel(2, ::atoi(message)); });
  mqtt.subscribe("ch3", [](const char* message){ b1.updateChannel(3, ::atoi(message)); });
  mqtt.subscribe("ch4", [](const char* message){ b1.updateChannel(4, ::atoi(message)); });
  mqtt.subscribe("ch5", [](const char* message){ b1.updateChannel(5, ::atoi(message)); });
#endif
  mqtt.subscribe("control", [](const char* message){
    sunrise.reset(); // no more sunrise !!
    if (0 == strcasecmp("ON", message))
    {
	    message = colorConfig.getLedColors();
	    colorConfig.setEnabled(true);
    }
    else if (0 == strcasecmp("OFF", message))
    {
	    message = "0";
	    colorConfig.setEnabled(false);
    }
    else if (0 == strcasecmp("toggle", message))
    {
      if (colorConfig.getEnabled())
      {
        colorConfig.setEnabled(false);
        message = "0";
      }
      else
      {
        colorConfig.setEnabled(true);
        message = colorConfig.getLedColors();
      }
    }
    else if (0 == strcasecmp("error", message))
    {
      b1.controlLeds("0,0,255,0,0");
      return;
    }
    else
    {
      colorConfig.setLedColors(message);
      colorConfig.setEnabled(true);
    }

    b1.controlLeds(message);
    colorConfig.save();
  });
#if defined (SUNRISE)
  mqtt.subscribe("sunrise", [](const char*message){
    uint32_t dt = ::atoi(message);
    if (dt > 0)
    {
      sunrise.start(dt);
    }
    else
    {
      b1.controlLeds("0");
    }
  });
#endif

  b1.controlLeds(colorConfig.getEnabled() ? colorConfig.getLedColors(): "0");
}

void loop() {
  tsystem.run_loop(1,1);
}
