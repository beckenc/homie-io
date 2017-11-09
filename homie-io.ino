#include <Homie.hpp>
#include <vector>
#include "Input.hpp"

//#define _DEBUG

static HomieSetting<bool> deepSleepSetting("deepSleep", "ESP deep sleep mode");
static HomieSetting<long> publishIntervalSetting("publishInterval", "publish interval in seconds");

static void setupHandler() {
  Homie.getLogger() << "homie-io - " << __DATE__ << " - " << __TIME__ << endl;
}

static void loopHandler() {
  if (deepSleepSetting.get()) {
    // publishing successful. Go into deep sleep.
#ifndef _DEBUG
    Homie.getLogger() << "Preparing for deep sleep (" << publishIntervalSetting.get() << " seconds)" << endl;
    Homie.prepareToSleep();
#endif
  }
}

static void onHomieEvent(const HomieEvent & event) {
  switch (event.type) {
    case HomieEventType::READY_TO_SLEEP:
      Homie.getLogger() << "Ready to sleep" << endl;
      // convert to microseconds
      ESP.deepSleep(publishIntervalSetting.get() * 1000000);
      break;
  }
}

void setup() {
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("homie-io", "0.0.1");
  Homie_setBrand("homie-io");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler).onEvent(onHomieEvent);
  Homie.disableLedFeedback();
  Homie.disableResetTrigger();

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
