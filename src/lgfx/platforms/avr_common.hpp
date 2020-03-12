#ifndef LGFX_AVR_COMMON_HPP_
#define LGFX_AVR_COMMON_HPP_

#include <Arduino.h>

namespace lgfx {

  template<uint8_t PIN>
  struct AVRPIN {
    static void init() {  }

    inline static void enableInput() { pinMode(PIN, INPUT); }
    inline static void enableOutput() __attribute__ ((always_inline)) { pinMode(PIN, OUTPUT); }
    inline static void disableOutput() __attribute__ ((always_inline)) { pinMode(PIN, INPUT); }
    inline static void hi() __attribute__ ((always_inline)) { digitalWrite(PIN, HIGH); }
    inline static void lo() __attribute__ ((always_inline)) { digitalWrite(PIN, LOW); }
//    inline static bool isset() __attribute__ ((always_inline)) { if (PIN < 32) return GPIO.out & MASK; else return GPIO.out1.val & MASK; }
  };
  struct AVRNOPIN {
  public:
    inline static void init() __attribute__ ((always_inline)) {}
    inline static void enableInput() __attribute__ ((always_inline)) {}
    inline static void enableOutput() __attribute__ ((always_inline)) {}
    inline static void disableOutput() __attribute__ ((always_inline)) {}
    inline static void hi() __attribute__ ((always_inline)) {}
    inline static void lo() __attribute__ ((always_inline)) {}
//    inline static bool isset() __attribute__ ((always_inline)) { return false; }
  };

  template<int PIN>
  struct TPin : public AVRPIN<PIN> {};

  template<>
  struct TPin<-1> : public AVRNOPIN {};


};

#endif
