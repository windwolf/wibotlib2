//
// Created by zhouj on 2023/9/22.
//

#include "message-reader.hpp"
#include "hal/system.hpp"
#include "type.hpp"

#include "logger.hpp"
LOGGER("msgreader")

namespace wibot {

namespace {
AsyncEventSource& requireSource(AsyncEventSource* source) {
    ASSERT(source != nullptr, "message reader source must not be null.");
    return *source;
}
}  // namespace

MessageReader::MessageReader(AsyncEventSource* source, CircularBuffer8& buffer,
                             const MessageSchema& schema, bool interFrameGap)
    : _source(requireSource(source)),
      _buffer(buffer),
      _interFrameGap(interFrameGap),
      _mp(schema, _buffer),
      _rxWaitHandler(AsyncResult::fromError(Result::kInvalidParameter)) {
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
Result MessageReader::read(MessageFrame& frame, u32 timeout) {
    u32    startTick = System::getTickMs();
    Result rst       = Result::kOk;

    rst = _mp.parse(&frame, _interFrameGap);
    if (rst == Result::kOk) {
        return Result::kOk;
    }

    do {
        auto duration = System::getDurationMs(startTick);
        if (duration < timeout) {
            rst = _rxWaitHandler.wait(timeout - duration);
        } else {
            rst = _rxWaitHandler.wait(TIMEOUT_NOWAIT);
        }
        if (rst.isError()) {
            auto stopRst = _source.stop();
            _buffer.clear();
            _mp.reset();
            if (!stopRst.isOk()) {
                return stopRst.isTimeout() ? Result::kTimeout : Result::kError;
            }
            _rxWaitHandler = _source.subscribe();
            auto startRst  = _source.start();
            if (!startRst.isOk()) {
                return startRst.isTimeout() ? Result::kTimeout : Result::kError;
            }
        } else if (rst.isTimeout()) {
            return Result::kTimeout;
        } else {
            break;
        }
    } while (true);

    return _mp.parse(&frame);
}

}  // namespace wibot
