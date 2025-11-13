#pragma once

//
// Created by zhouj on 2024/3/30.
//

#include "type.hpp"

namespace wibot {
#ifdef HAL_FLASH_MODULE_ENABLED
class HalFlash {
   public:
    Result      writePage(u32 address, const void* data, u32 size);
    const void* read(u32 address, u32 size);
    Result      erasePage(u32 address, u32 size);

   private:
};

class HalCachedFlash {
   public:
    Result      write(u32 address, const void* data, u32 size);
    const void* read(u32 address, u32 size);
    Result      erase(u32 address, u32 size);

   private:
    u8 _pageCache[kPageSize];
};
#endif  // HAL_FLASH_MODULE_ENABLED
}  // namespace wibot
