#include "button.h"

namespace tc {

Press Button::update(bool pressed, uint32_t nowMs) {
    if (pressed != raw_) {
        raw_ = pressed;
        rawChangedAt_ = nowMs;
    }
    if (raw_ != stable_ && nowMs - rawChangedAt_ >= kDebounceMs) {
        stable_ = raw_;
        if (stable_) {
            pressedAt_ = nowMs;
            longFired_ = false;
        } else if (!longFired_) {
            return Press::Short;
        }
    }
    if (stable_ && !longFired_ && nowMs - pressedAt_ >= kLongPressMs) {
        longFired_ = true;
        return Press::Long;
    }
    return Press::None;
}

}  // namespace tc
