#ifndef _INPUT_NODE
#define _INPUT_NODE

#include <Bounce2.h>

class Input : HomieNode {
  public:
    Input(HomieSetting<long> &pubInterval, int inputPin, const char* id);
    virtual ~Input();

    void setup();
    void onReadyToOperate();
    void loop();
    bool publish();

  private:
    String alias;
    Bounce debouncer;
    HomieSetting<long> &publishInterval;
    HomieSetting<const char*> aliasSetting;    
    const int pin;
    unsigned long publishTimeStamp;
};

inline Input::Input(HomieSetting<long> &pubInterval, int inputPin, const char* id)
  : HomieNode(id, "input")
  , alias(String("alias-") + id)
  , debouncer()
  , publishInterval(pubInterval)
  , aliasSetting(alias.c_str(), "Alias of the device_id")
  , pin(inputPin)
  , publishTimeStamp(0) {
  pinMode(pin, INPUT_PULLUP);
  aliasSetting.setDefaultValue(id);
}

inline Input::~Input() {
}

inline void Input::setup() {
  debouncer.attach(pin);
  debouncer.interval(100);
  debouncer.update();
  advertise("state");
  advertise("alias");
}

inline void Input::onReadyToOperate() {
  setProperty("alias").setRetained(true).send(aliasSetting.get());
  publish();
}

inline void Input::loop() {
  if (debouncer.update() || ((millis() - publishTimeStamp) > (publishInterval.get() * 1000UL)) ) {
    publish();
  }
}

bool Input::publish() {
  publishTimeStamp = millis();
  
  return (setProperty("state").setRetained(true).send(String(debouncer.read())) != 0);
}

#endif
