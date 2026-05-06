#include "rx-server.hpp"
#include "buffer.hpp"

#include "logger.hpp"
#include "type.hpp"

LOGGER("rxsvr")

namespace wibot {
RxServer::RxServer(MessageReader& reader, u32 timeoutMs) : _reader(reader), _timeoutMs(timeoutMs) {
}

RxServer::~RxServer() {
}

void RxServer::run() {
    Buffer<MAX_RX_SERVER_FRAME_SIZE> _frameBuffer;
    MessageFrame                     frame(_frameBuffer);
    Result                           rst;

    rst = startServer(true);
    ASSERT(!rst.isError(), "Failed to start command server. Error code: %u",
           (unsigned int)rst.getErrorCode());

    while (true) {
        rst = _reader.read(frame, _timeoutMs);
        if (rst.isOk()) {
            if (validateFrame(frame)) {
                processCommandFrame(frame);
            } else {
                LOG_W("Invalid frame received.");
            }
        } else if (rst.isError()) {
            LOG_E("Error occurred when reading message frame: %u",
                  (unsigned int)rst.getErrorCode());
            stopServer(true);
            startServer(true);
        } else if (rst.isTimeout()) {
        }
    }
}

Result RxServer::startServer(bool retry) {
    Result rst;
    do {
        rst = _reader.open();
        if (!rst.isOk()) {
            if (retry) {
                LOG_E("Failed to start serve. Retrying...");
            } else {
                LOG_E("Failed to start server.");
                return rst;
            }
            sleep(1);
        }
    } while (!rst.isOk() && retry);
    LOG_I("Command server started.");
    return rst;
}

Result RxServer::stopServer(bool retry) {
    Result rst;
    do {
        if (!rst.isOk()) {
            if (retry) {
                LOG_E("Failed to stop serve. Retrying...");
            } else {
                LOG_E("Failed to stop server.");
                return rst;
            }
        }
        rst = _reader.close();
    } while (!rst.isOk() && retry);
    LOG_I("Command server stopped.");
    return rst;
}

void RxServer::setTimeout(u32 timeoutMs) {
    _timeoutMs = timeoutMs;
}
}  // namespace wibot
