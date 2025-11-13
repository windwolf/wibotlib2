#include "command.hpp"

namespace wibot {

Command::Command(u32 timeout) : _timeout(timeout) {};

Result Command::send(CommandFrame &frame, WaitHandler &waitHandler) {
    Result rst;

    WaitHandler wh = WaitHandler();
    do {
        rst = mediaBeginSession(wh);
        if (rst != Result::kOk) {
            break;
        }
        rst = wh.wait(_timeout);
        if (rst != Result::kOk) {
            break;
        }
        rst = mediaSendCommand(frame, wh);
        if (rst != Result::kOk) {
            break;
        }
        rst = wh.wait(_timeout);
        if (rst != Result::kOk) {
            break;
        }

    } while (0);

    // TODO: error handler.
    mediaEndSession(wh);
    wh.wait(_timeout);

    WaitTrigger wt;
    wt.attach(waitHandler);
    wt.setDone();
    return rst;
}

}  // namespace wibot
