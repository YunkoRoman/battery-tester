#pragma once
#include <stdint.h>

namespace tc {

enum class Press : uint8_t { None, Short, Long };

const uint32_t kDebounceMs = 30;
const uint32_t kLongPressMs = 800;

class Button {
  public:
    // pressed: raw pin state (true = button down). Call every loop iteration.
    Press update(bool pressed, uint32_t nowMs);

  private:
    bool raw_ = false;
    bool stable_ = false;
    bool longFired_ = false;
    uint32_t rawChangedAt_ = 0;
    uint32_t pressedAt_ = 0;
};

}  // namespace tc
