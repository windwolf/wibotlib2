//
// Created by zhouj on 2023/9/8.
//

#include "math.hpp"

namespace wibot {

u32 Math::fastLog2(u32 val) {
    u32          ret;
    f32          fdata = (f32)val;
    // unsigned int uData = (fdata>>23)&0xFF;
    // //ç´ćĽç§ťĺ¨ĺşé
    u32          data  = *(u32 *)&fdata;  //(unsigned int&)fdata ä¸(unsigned
                                          // int*)&fdata; ä¸č
    unsigned int udata = (data >> 23) & 0xFF;
    ret                = (int)udata - 127;  //-ć çŹŚĺˇĺ°ćçŹŚĺ

    return ret;
};

}  // namespace wibot
