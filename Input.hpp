#ifndef _INPUT_NODE
#define _INPUT_NODE

class Input : HomieNode {
  public:
    Input(int input, const char* id);
    virtual ~Input();

    struct state {
      unsigned long millis;
      bool state;
    };

    void setup();
    state current() const;
    state last() const;
    bool publish();
 
 private:
    int pin;
    state lastState;
};

inline Input::Input(int input, const char* id)
  : HomieNode(id, "input")
  , pin(input) {
  pinMode(pin, INPUT_PULLUP);
  lastState.state = digitalRead(pin) == 0;
  lastState.millis = 0;
}

inline Input::~Input() {
}

inline void Input::setup() {
  advertise("state");
}

Input::state Input::current() const {
  return { millis(), digitalRead(pin) == 0 };
}

Input::state Input::last() const {
  return lastState;
}

bool Input::publish() {
  lastState.state = digitalRead(pin) == 0;
  lastState.millis = millis();
  return setProperty("state").setRetained(true).send(String(lastState.state)) != 0;
}

#endif
