#pragma once

#include "type.hpp"
#include "buffer.hpp"
#include "os.hpp"
#include "wait-handler.hpp"
namespace wibot {

enum class CommandFrameMode {
    kSkip   = 0,
    k1Line  = 1,
    k2Line  = 2,
    k4Lines = 3,
};

enum class CommandFramePhase {
    kCommand,
    kAddress,
    kAltData,
    kDummyCycle,
    kData,
};
struct CommandFrame {
   public:
    ALIGN32 u8 commandId;
    u32        address;
    u32        altData;
    void      *data;
    u16        dataSize;
    struct {
        CommandFrameMode commandMode : 2;
        // u32 commandBits : 2; // always 8bits

        CommandFrameMode addressMode : 2;
        DataWidth        addressBits : 3;

        CommandFrameMode altDataMode : 2;
        DataWidth        altDataBits : 3;

        CommandFrameMode dataMode : 2;
        DataWidth        dataBits : 3;
        u32              isWrite  : 1;

        u32 dummyCycles : 5;

        u32 isDdr : 1;
        u32       : 8;
    };
};

class Command {
   public:
    Command(u32 timeout);
    Result send(CommandFrame &frame, WaitHandler &waitHandler);

   protected:
    virtual Result mediaBeginSession(WaitHandler &waitHandler)                     = 0;
    virtual Result mediaEndSession(WaitHandler &waitHandler)                       = 0;
    virtual Result mediaSendCommand(CommandFrame &frame, WaitHandler &waitHandler) = 0;

   protected:
    u32 _timeout;

   private:
    //u32 _readyFlag;
};

}  // namespace wibot
