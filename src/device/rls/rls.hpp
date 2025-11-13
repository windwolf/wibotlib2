#pragma once

#include "type.hpp"

namespace wibot {
class Rls {
   public:
    virtual u32 getAngle() = 0;
    virtual u32 getData()  = 0;
};
}  // namespace wibot
