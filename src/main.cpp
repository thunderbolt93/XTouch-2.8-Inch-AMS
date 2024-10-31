#include <driver/i2s.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "xtouch/debug.h"
#include "xtouch/paths.h"
#include "xtouch/eeprom.h"
#include "xtouch/types.h"
#include "xtouch/bblp.h"
#include "xtouch/globals.h"
#include "xtouch/filesystem.h"
#include "ui/ui.h"
#include "xtouch/sdcard.h"
#include "xtouch/hms.h"

#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#endif

#include "xtouch/cloud.hpp"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/coldboot.h"

void xtouch_intro_show(void)
{
  ui_introScreen_screen_init();
  lv_disp_load_scr(introScreen);
  lv_timer_handler();
}

void setup()
{

#if XTOUCH_USE_SERIAL == true || XTOUCH_DEBUG_ERROR == true || XTOUCH_DEBUG_DEBUG == true || XTOUCH_DEBUG_INFO == true
  Serial.begin(115200);
#endif

  xtouch_eeprom_setup();
  xtouch_globals_init();
  xtouch_screen_setup();
  xtouch_intro_show();
  while (!xtouch_sdcard_setup())
    ;

  xtouch_coldboot_check();

  xtouch_settings_loadSettings();

  xtouch_firmware_checkFirmwareUpdate();

  xtouch_touch_setup();

  while (!xtouch_wifi_setup())
    ;

  xtouch_firmware_checkOnlineFirmwareUpdate();

  xtouch_screen_setupScreenTimer();
  xtouch_setupGlobalEvents();
  if (cloud.login())
  {
    if (!cloud.isPaired())
    {
      cloud.selectPrinter();
    }
    else
    {
      cloud.loadPair();
    }
  }
  xtouch_mqtt_setup();
  xtouch_chamber_timer_init();

  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
    })
    .onEnd([]() {
     
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      uint8_t prog = (progress / (total / 100));
     

    })
    .onError([](ota_error_t error) {
      
      // if (error == OTA_AUTH_ERROR) dma_display->print(" Auth Failed");
      // else if (error == OTA_BEGIN_ERROR) dma_display->print(" Begin Failed");
      // else if (error == OTA_CONNECT_ERROR) dma_display->print(" Connect Failed");
      // else if (error == OTA_RECEIVE_ERROR) dma_display->print(" Receive Failed");
      // else if (error == OTA_END_ERROR) dma_display->print(" End Failed");

      
      delay(1000);
      ESP.restart();
    });
  
  ArduinoOTA.begin();


}

void loop()
{
  lv_timer_handler();
  lv_task_handler();
  xtouch_mqtt_loop();
  ArduinoOTA.handle();
}
