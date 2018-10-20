#include <Homie.hpp>
#include <vector>
#include "Input.hpp"

//#define _DEBUG

static HomieSetting<bool> deepSleepSetting("deepSleep", "ESP deep sleep mode");
static HomieSetting<long> publishIntervalSetting("publishInterval", "publish interval in seconds");
static unsigned long awakeTillMillis = 0;

static void setupHandler() {
  Homie.getLogger() << "homie-io - " << __DATE__ << " - " << __TIME__ << endl;
  awakeTillMillis = millis() + 1000UL; // stay awake for at least one second to give pending broadcasts a
                                       // chance to arrive before re-entering sleep
}

bool broadcastHandler(const String& level, const String& value) {
  if(level == "wakeup") {
    long awakeTillSec = value.toInt();
    if(0 != awakeTillSec) {
      awakeTillMillis = millis() + (awakeTillSec * 1000UL);
      Homie.getLogger() << "awake for the next " << awakeTillSec << " seconds" << endl;
    }
    return true;
  }
  return false;
}

static void loopHandler() {
  if (deepSleepSetting.get()) {
    if(millis() >= awakeTillMillis) {
      // publishing successful. Go into deep sleep.
#ifndef _DEBUG
      static bool prepared = false;
      if(!prepared) {
        prepared = true;
        Homie.getLogger() << "Preparing for deep sleep (" << publishIntervalSetting.get() << " seconds)" << endl;
        Homie.prepareToSleep();
      }
#endif
    }
  }
}

void onHomieEvent(const HomieEvent& event) {
  static bool readyToSleep = false;
  switch (event.type) {
    case HomieEventType::READY_TO_SLEEP:
      WiFi.disconnect();    // Calling doDeepSleep() at this point sometimes causes exceptions due to asynchronous arrival 
                            // of WIFI_DISCONNECTED. WDT wakes up the Device immediatly after the exception. So deep sleep does not work.
                            // To work around this, I decided to inherit this task to the WIFI_DISCONNECTED event.
      readyToSleep = true;
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      if(readyToSleep)
        Homie.doDeepSleep(publishIntervalSetting.get() * 1000000);
      break;
  }
}

void setup() {
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("homie-io", "0.0.2");
  Homie_setBrand("homie-io");
  Homie.setSetupFunction(setupHandler).setBroadcastHandler(broadcastHandler).setLoopFunction(loopHandler).onEvent(onHomieEvent);
  Homie.disableLedFeedback();
  /* Homie.disableResetTrigger(); */ // disabled to support $implementation/reset -> true

  deepSleepSetting.setDefaultValue(false);
  publishIntervalSetting.setDefaultValue(300UL).setValidator([] (long candidate) {
    return (candidate >= 2) && (candidate < 86400); // 2sec - 1day
  });

  // todo take the config from a HomieSettings Object
  new Input(publishIntervalSetting, D1, "in-0");
  new Input(publishIntervalSetting, D2, "in-1");
  new Input(publishIntervalSetting, D6, "in-2");
  new Input(publishIntervalSetting, D7, "in-3");

  Homie.setup();
}

void loop() {
  Homie.loop();
}
