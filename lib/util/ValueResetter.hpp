#pragma once

namespace qscan::lib {

template <class T>
class ValueResetter {
  private:
    T *valPtr;
    T  oldValue;

  public:
    ValueResetter(T *_valPtr, T _newValue) : valPtr(_valPtr), oldValue(*valPtr) { *valPtr = _newValue; }
    ~ValueResetter() { resetNow(); }

    inline void resetNow() {
        if (!valPtr) {
            return;
        }
        *valPtr = oldValue;
        valPtr  = nullptr;
    }

    [[nodiscard]] bool hasReset() const noexcept { return valPtr == nullptr; }

    // No moving and copying around please
    ValueResetter(const ValueResetter &) = delete;
    ValueResetter(ValueResetter &&)      = delete;

    ValueResetter &operator=(const ValueResetter &) = delete;
    ValueResetter &operator=(ValueResetter &&)      = delete;
};

} // namespace qscan::lib
