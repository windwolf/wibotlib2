#include "rx-server.hpp"
#include "buffer.hpp"

#include "logger.hpp"
LOGGER("cmdsrv")

namespace wibot {
RxServer::RxServer(MessageReader& reader) : _reader(reader) {
}

RxServer::~RxServer() {
}

void RxServer::run() {
    Buffer<MAX_RX_SERVER_FRAME_SIZE> _frameBuffer;
    MessageFrame                     frame(_frameBuffer);
    Result                           rst;

    startServer(true);

    while (true) {
        rst = _reader.read(frame, _timeoutMs);
        if (rst.isOk()) {
            if (validateFrame(frame)) {
                processCommandFrame(frame);
            } else {
                LOG_W("Invalid frame received.");
            }
        } else if (rst.isError()) {
            LOG_E("Error occurred when reading message frame: %d", rst.isError());
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
