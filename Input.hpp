#ifndef _INPUT_NODE
#define _INPUT_NODE

#include <Bounce2.h>

class Input : HomieNode {
  public:
    Input(int input, const char* id);
    virtual ~Input();

    void setup();
    bool update();
    bool publish();
    unsigned long lastPublish() const;

  private:
    const int pin;
    unsigned long publishTimeStamp;
    Bounce debouncer;
};

inline Input::Input(int input, const char* id)
  : HomieNode(id, "input")
  , pin(input) {
  pinMode(pin, INPUT_PULLUP);
}

inline Input::~Input() {
}

inline void Input::setup() {
  debouncer.attach(pin);
  debouncer.interval(100);
  advertise("state");
}

inline bool Input::update() {
  // return true if the state has changed and false if not
  return debouncer.update();
}

bool Input::publish() {
  publishTimeStamp = millis();
  return setProperty("state").setRetained(true).send(String(debouncer.read())) != 0;
}

unsigned long Input::lastPublish() const {
  return publishTimeStamp;
}

#endif
