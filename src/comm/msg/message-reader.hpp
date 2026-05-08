#pragma once

//
// Created by zhouj on 2023/9/22.
//
#include "bus.hpp"
#include "message-parser.hpp"

namespace wibot {

class MessageReader : public SyncReader<MessageFrame> {
   public:
    MessageReader(AsyncEventSource* source, CircularBuffer8& buffer, const MessageSchema& schema,
                  bool interFrameGap = false, FeedEvent feedEvents = FeedEvent::kFeedOnAll);

    Result open();
    Result read(MessageFrame& frame, u32 timeout);
    // Result writeSync(MessageFrame& frame);
    // Result writeSync(const Slice& buffer);
    // Result writeAsync(MessageFrame& frame, WaitHandler& waitHandler);
    // Result writeAsync(const Slice& buffer, WaitHandler& waitHandler);
    Result close();

   private:
    AsyncEventSource& _source;
    CircularBuffer8&  _buffer;
    const bool        _interFrameGap;
    const FeedEvent   _feedEvents;
    MessageParser     _mp;
    AsyncResult       _rxWaitHandler;
};

}  // namespace wibot
