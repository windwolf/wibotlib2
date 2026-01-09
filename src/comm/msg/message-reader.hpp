#pragma once

//
// Created by zhouj on 2023/9/22.
//
#include "bus.hpp"
#include "message-parser.hpp"

namespace wibot::comm {

class MessageReader : public SyncReader<MessageFrame> {
   public:
    MessageReader(AsyncEventSource* source, CircularBuffer8& buffer, const MessageSchema& schema,
                  bool interFrameGap = false);

    Result open();
    Result read(comm::MessageFrame& frame, u32 timeout);
    // Result writeSync(comm::MessageFrame& frame);
    // Result writeSync(const Slice& buffer);
    // Result writeAsync(comm::MessageFrame& frame, WaitHandler& waitHandler);
    // Result writeAsync(const Slice& buffer, WaitHandler& waitHandler);
    Result close();

   private:
    AsyncEventSource& _source;
    CircularBuffer8&  _buffer;
    const bool        _interFrameGap;
    MessageParser     _mp;
    os::AsyncResult   _rxWaitHandler;
};

}  // namespace wibot::comm
