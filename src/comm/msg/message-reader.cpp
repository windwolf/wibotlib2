//
// Created by zhouj on 2023/9/22.
//

#include "message-reader.hpp"
#include "hal/system.hpp"
#include "type.hpp"

namespace wibot::comm {

using namespace hal;

MessageReader::MessageReader(AsyncEventSource* source, CircularBuffer8& buffer,
                             const MessageSchema& schema, bool interFrameGap)
    : _source(*source),
      _buffer(buffer),
      _interFrameGap(interFrameGap),
      _mp(schema, _buffer),
      _rxWaitHandler(os::AsyncResult::fromError(Result::kInvalidParameter)) {
}
Result MessageReader::open() {
    _buffer.clear();
    _mp.reset();
    _rxWaitHandler = _source.subscribe();
    return _source.start();
}
Result MessageReader::close() {
    _buffer.clear();
    _mp.reset();
    return _source.stop();
}
Result MessageReader::read(comm::MessageFrame& frame, u32 timeout) {
    u32    startTick = hal::System::getTickMs();
    Result rst       = Result::kOk;

    rst = _mp.parse(&frame, _interFrameGap);
    if (rst == Result::kOk) {
        return Result::kOk;
    }

    do {
        auto duration = hal::System::getDurationMs(startTick);
        if (duration < timeout) {
            rst = _rxWaitHandler.wait(timeout - duration);
        } else {
            rst = _rxWaitHandler.wait(TIMEOUT_NOWAIT);
        }
        if (rst == Result::kError) {
            _source.stop();
            _buffer.clear();
            _mp.reset();
            _source.start();
        } else if (rst == Result::kTimeout) {
            return Result::kTimeout;
        } else {
            break;
        }
    } while (true);

    return _mp.parse(&frame);
}

}  // namespace wibot::comm
