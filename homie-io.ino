#include <Homie.hpp>
#include <vector>
#include "Input.hpp"

#define _DEBUG

static HomieSetting<long> publishIntervalSetting("publishInterval", "publish interval in seconds");
static HomieSetting<bool> deepSleepSetting("deepSleep", "ESP deep sleep mode");
static std::vector<Input*> inputs;

static void setupHandler() {
  Homie.getLogger() << "homie-io - " << __DATE__ << " - " << __TIME__ << endl;
  for (std::vector<Input*>::iterator input = inputs.begin() ; input != inputs.end(); ++input) {
    (*input)->setup();
  }
}

static void loopHandler() {
  const bool deepSleep = deepSleepSetting.get();
  const long publishInterval = publishIntervalSetting.get();

  for (std::vector<Input*>::iterator input = inputs.begin() ; input != inputs.end(); ++input) {
    if ((*input)->update() || ((millis() - (*input)->lastPublish()) > publishInterval) ) {
      (*input)->publish();
    }
  }

  if (deepSleep) {
    // publishing successful. Go into deep sleep.
#ifndef _DEBUG
    Homie.getLogger() << "Preparing for deep sleep (" << publishInterval << " seconds)" << endl;
    Homie.prepareToSleep();
#endif
  }
}

void onHomieEvent(const HomieEvent & event) {
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
  inputs.push_back(new Input(D1, "in[0]"));
  inputs.push_back(new Input(D2, "in[1]"));
  inputs.push_back(new Input(D6, "in[2]"));
  inputs.push_back(new Input(D7, "in[3]"));

  Homie.setup();
}

void loop() {
  Homie.loop();
}
