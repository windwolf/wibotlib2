#include "message-parser.hpp"
#include "arch.hpp"

#include "logger.hpp"
LOGGER("mp")

namespace wibot {

u32 MessageSchema::getContentOverhead(const MessageLengthSchema* lengthSchema) const {
    u32 oh = 0;
    if (lengthSchema->mode == MessageLengthSchemaMode::kFixedLength) {
        oh += prefixSize;
        oh += static_cast<u8>(commandSize);
        oh += static_cast<u8>(alterDataSize);
        oh += static_cast<u8>(crcSize);
        oh += suffixSize;
    } else if (lengthSchema->mode == MessageLengthSchemaMode::kDynamicLength) {
        oh += prefixSize;
        oh += static_cast<u8>(commandSize);
        oh += static_cast<u8>(lengthSchema->dynamic.lengthSize);
        oh += static_cast<u8>(alterDataSize);
        oh += static_cast<u8>(crcSize);
        oh += suffixSize;
    } else if (lengthSchema->mode == MessageLengthSchemaMode::kFreeLength) {
        oh += prefixSize;
        oh += static_cast<u8>(commandSize);
        oh += static_cast<u8>(alterDataSize);
        oh += suffixSize;
    }
    return oh;
}
u32 MessageSchema::getDynamicLengthOverhead(const MessageLengthSchema* lengthSchema) const {
    u32 oh = 0;
    if (lengthSchema->mode == MessageLengthSchemaMode::kDynamicLength) {
        auto lengthRange = lengthSchema->dynamic.range;
        if (lengthRange & kMessageSchemaRangePrefix) {
            oh += prefixSize;
        }
        if (lengthRange & kMessageSchemaRangeCmd) {
            oh += static_cast<u8>(commandSize);
        }
        if (lengthRange & kMessageSchemaRangeLength) {
            oh += static_cast<u8>(lengthSchema->dynamic.lengthSize);
        }
        if (lengthRange & kMessageSchemaRangeAlterdata) {
            oh += static_cast<u8>(alterDataSize);
        }
        if (lengthRange & kMessageSchemaRangeCrc) {
            oh += static_cast<u8>(crcSize);
        }
        if (lengthRange & kMessageSchemaRangeSuffix) {
            oh += suffixSize;
        }
    }
    return oh;
}
u32 MessageSchema::getLength(const MessageLengthSchema* lengthSchema, u32 contentLength) const {
    if (lengthSchema->mode == MessageLengthSchemaMode::kFixedLength) {
        return getContentOverhead(lengthSchema) + lengthSchema->fixed.length;
    } else if (lengthSchema->mode == MessageLengthSchemaMode::kDynamicLength) {
        return contentLength + getContentOverhead(lengthSchema);
    } else {
        return contentLength + getContentOverhead(lengthSchema);
    }
}
const MessageLengthSchema& MessageSchema::getLengthSchema(const u8* command) const {
    auto cmdSize = static_cast<u8>(this->commandSize);
    if (cmdSize == 0) {
        return defaultLength;
    }

    for (u32 i = 0; i < lengthSchemaCount; ++i) {
        if (memcmp(lengthSchemas[i].command, command, cmdSize) == 0) {
            return lengthSchemas[i].length;
        }
    }
    return defaultLength;
}

MessageFrame::MessageFrame(Slice buffer)
    : _prefix({0}),
      _command({0}),
      _length({0}),
      _alterData({0}),
      _content({0}),
      _crc({0}),
      _suffix({0}),
      _buffer(buffer) {
}

MessageFrame::MessageFrame(Slice buffer, const MessageSchema& schema, u8* command,
                           u32 contentLength)
    : _buffer(buffer) {
    auto lengthSchema = schema.getLengthSchema(command);
    ASSERT(_frameLength <= buffer.size, "buffer size is not enough.");

    _prefix.offset = 0;
    _prefix.length = schema.prefixSize;

    _command.offset = _prefix.offset + _prefix.length;
    _command.length = static_cast<u8>(schema.commandSize);

    _length.offset    = _command.offset + _command.length;
    _length.length    = (lengthSchema.mode == MessageLengthSchemaMode::kDynamicLength)
                            ? static_cast<u8>(lengthSchema.dynamic.lengthSize)
                            : 0;
    _alterData.offset = _length.offset + _length.length;
    _alterData.length = static_cast<u8>(schema.alterDataSize);

    _content.offset = _alterData.offset + _alterData.length;
    _content.length = (lengthSchema.mode == MessageLengthSchemaMode::kFixedLength)
                          ? static_cast<u8>(lengthSchema.fixed.length)
                          : contentLength;

    _crc.offset = _content.offset + _content.length;
    _crc.length = static_cast<u8>(schema.crcSize);

    _suffix.offset = _crc.offset + _crc.length;
    _suffix.length = schema.suffixSize;

    _frameLength = schema.getLength(&lengthSchema, contentLength);
}
Slice MessageFrame::getPrefix() const {
    return Slice(this->_buffer.data + this->_prefix.offset, this->_prefix.length);
}
Slice MessageFrame::getCommand() const {
    return Slice(this->_buffer.data + this->_command.offset, this->_command.length);
}
Slice MessageFrame::getLength() const {
    return Slice(this->_buffer.data + this->_length.offset, this->_length.length);
}
Slice MessageFrame::getAlterdata() const {
    return Slice(this->_buffer.data + this->_alterData.offset, this->_alterData.length);
}
Slice MessageFrame::getContent() const {
    return Slice(this->_buffer.data + this->_content.offset, this->_content.length);
}
Slice MessageFrame::getCrc() const {
    return Slice(this->_buffer.data + this->_crc.offset, this->_crc.length);
}
Slice MessageFrame::getSuffix() const {
    return Slice(this->_buffer.data + this->_suffix.offset, this->_suffix.length);
}
const Slice& MessageFrame::getWholeBuffer() const {
    return this->_buffer;
}
Slice MessageFrame::buildFrame(const MessageSchema& schema) {
    //auto lengthSchema = schema.getLengthSchema(_buffer.data + _command.offset);
    //ASSERT(_frameLength <= _buffer.size, "buffer size is not enough.");
    if (_prefix.length > 0) {
        memcpy(_buffer.data + _prefix.offset, schema.prefix, _prefix.length);
    }
    if (_suffix.length > 0) {
        memcpy(_buffer.data + _suffix.offset, schema.suffix, _suffix.length);
    }
    return getWholeBuffer();
}

MessageParser::MessageParser(const MessageSchema& schema, CircularBuffer8& buffer)
    : _schema(schema), _buffer(buffer) {
    _checkSchema();
}

Result MessageParser::parse(MessageFrame* parsedFrame, bool interFrameGap) {
    if (parsedFrame == nullptr) {
        return Result::kInvalidParameter;
    }
    if (interFrameGap) {
        auto length = this->_schema.getLength(&_schema.defaultLength, 0);
        if (_buffer.getSize() < length) {
            return Result::kNoResource;
        }
        if (_buffer.getSize() > length) {
            _buffer.readVirtual(_buffer.getSize() - length);
        }
    }

    MessageParseStage stage       = _stage;
    auto              needNewEpic = false;
    if (_frame != parsedFrame) {
        _frame = parsedFrame;
        stage  = MessageParseStage::kInit;
    }

    do {
        if (stage == MessageParseStage::kInit) {
            _offset = 0;
            stage   = MessageParseStage::kPreparing;
        }
        if (stage == MessageParseStage::kPreparing) {
            _prepareFrame();

            stage = MessageParseStage::kSeekingPrefix;
        }
        if (stage == MessageParseStage::kSeekingPrefix) {
            if (_schema.prefixSize > 0) {
                _frame->_prefix.offset = 0;
                auto result            = _seek(_schema.prefix, _schema.prefixSize);
                if (result) {
                    // found prefix
                    _remove(_offset);
                    _move(_schema.prefixSize);

                    _frame->_prefix.length = _schema.prefixSize;
                    stage                  = MessageParseStage::kParsingCmd;

                } else {
                    // not found prefix
                    _remove(_offset);
                    // stay in this stage, and wait for more data.
                }
            } else {
                stage = MessageParseStage::kParsingCmd;
            }
        }

        if (stage == MessageParseStage::kParsingCmd) {
            if (static_cast<u8>(_schema.commandSize) > 0) {
                _frame->_command.offset = _offset;
                auto result             = _fetch(_command, static_cast<u8>(_schema.commandSize));
                if (result) {
                    _frame->_command.length = static_cast<u8>(_schema.commandSize);
                    stage                   = MessageParseStage::kParsingLength;
                } else {
                    // Not enough data to parse command, stay in this stage.
                }
            } else {
                stage = MessageParseStage::kParsingLength;
            }
        }

        if (stage == MessageParseStage::kParsingLength) {
            _lengthSchema    = _lengthSchemaMatch();
            _contentOverhead = _schema.getContentOverhead(_lengthSchema);

            if (_lengthSchema->mode == MessageLengthSchemaMode::kFixedLength) {
                _contentLength = _lengthSchema->fixed.length;
                if ((_contentLength + _contentOverhead) > _frame->_buffer.size) {
                    _buffer.readVirtual(1);
                    _offset     = 0;
                    stage       = MessageParseStage::kPreparing;
                    needNewEpic = true;
                } else {
                    stage = MessageParseStage::kParsingAlterdata;
                }
            } else if (_lengthSchema->mode == MessageLengthSchemaMode::kDynamicLength) {
                _frame->_length.offset = _offset;
                u8 lengthBuf[kMessageParserCmdLengthCrcBufferSize];

                auto result = _fetch(lengthBuf, static_cast<u8>(_lengthSchema->dynamic.lengthSize));
                if (result) {
                    auto lengthOverhead = _schema.getDynamicLengthOverhead(_lengthSchema);
                    _contentLength      = _parseLength(_lengthSchema, lengthBuf) - lengthOverhead;
                    // check length limitation.
                    if ((_contentLength + _contentOverhead) > _frame->_buffer.size) {
                        _buffer.readVirtual(1);
                        _offset     = 0;
                        stage       = MessageParseStage::kPreparing;
                        needNewEpic = true;
                    } else {
                        _frame->_length.length = static_cast<u8>(_lengthSchema->dynamic.lengthSize);
                        stage                  = MessageParseStage::kParsingAlterdata;
                    }
                } else {
                    // Not enough data to parse length, stay in this stage.
                }
            } else {
                // free length mode, no length field.
                stage = MessageParseStage::kParsingAlterdata;
            }
        }

        if (stage == MessageParseStage::kParsingAlterdata) {
            if (static_cast<u8>(_schema.alterDataSize) > 0) {
                _frame->_alterData.offset = _offset;
                auto result               = _move(static_cast<u8>(_schema.alterDataSize));
                if (result) {
                    _frame->_alterData.length = static_cast<u8>(_schema.alterDataSize);
                    stage                     = MessageParseStage::kSeekingContent;
                } else {
                    // Not enough data to parse command, stay in this stage.
                }
            } else {
                stage = MessageParseStage::kSeekingContent;
            }
        }

        if (stage == MessageParseStage::kSeekingContent) {
            if (_lengthSchema->mode != MessageLengthSchemaMode::kFreeLength) {
                if (_contentLength > 0) {
                    _frame->_content.offset = _offset;
                    auto result             = _move(_contentLength);
                    if (result) {
                        _frame->_content.length = _contentLength;
                        stage                   = MessageParseStage::kSeekingCrc;
                    } else {
                        // Not enough data for content, stay in this stage.
                    }
                } else {
                    stage = MessageParseStage::kSeekingCrc;
                }
            } else {
                // free mode
                // record the start index.
                _freeContentStartIndex  = _offset;
                _frame->_content.offset = _freeContentStartIndex;
                // not support crc, so skip crc stage.
                stage                   = MessageParseStage::kMatchingSuffix;
            }
        }

        if (stage == MessageParseStage::kSeekingCrc) {
            if (static_cast<u8>(_schema.crcSize) > 0) {
                _frame->_crc.offset = _offset;
                auto result         = _move(static_cast<u8>(_schema.crcSize));
                if (result) {
                    _frame->_crc.length = static_cast<u8>(_schema.crcSize);
                    // TODO: CRC
                    stage               = MessageParseStage::kMatchingSuffix;
                } else {
                    // Not enough data for crc, stay in this stage.
                }
            } else {
                stage = MessageParseStage::kMatchingSuffix;
            }
        }

        if (stage == MessageParseStage::kMatchingSuffix) {
            if (_lengthSchema->mode != MessageLengthSchemaMode::kFreeLength) {
                if (_schema.suffixSize > 0) {
                    _frame->_suffix.offset = _offset;
                    auto result            = _match(_schema.suffix, _schema.suffixSize);
                    if (result == -1) {
                        // not enough buffer, stay in this stage.

                    } else if (result == 1) {
                        // success
                        _frame->_suffix.length = _schema.suffixSize;
                        stage                  = MessageParseStage::kDone;
                    } else {
                        // mismatch
                        // discard one data that has been parsed.
                        _buffer.readVirtual(1);
                        _offset     = 0;
                        stage       = MessageParseStage::kPreparing;
                        needNewEpic = true;
                    }
                } else {
                    stage = MessageParseStage::kDone;
                }
            } else {
                // free mode
                auto result = _seek(_schema.suffix, _schema.suffixSize);
                if (_offset - _freeContentStartIndex + _contentOverhead > _frame->_buffer.size) {
                    // discard one data that has been parsed.
                    _buffer.readVirtual(1);
                    _offset     = 0;
                    stage       = MessageParseStage::kPreparing;
                    needNewEpic = true;
                }
                if (result) {
                    _contentLength          = _offset - _freeContentStartIndex;
                    _frame->_content.length = _contentLength;
                    _frame->_suffix.offset  = _offset;
                    _move(_schema.suffixSize);
                    _frame->_suffix.length = _schema.suffixSize;
                    stage                  = MessageParseStage::kDone;
                } else {
                    // suffix not found, stay in this stage.
                }
            }
        }

        if (stage == MessageParseStage::kDone) {
            _frame->_frameLength = _offset;
            _buffer.read(_frame->_buffer.data, _offset);
            _offset = 0;
            stage   = MessageParseStage::kPreparing;

            _stage = stage;
            return Result::kOk;
        }

    } while (needNewEpic);

    _stage = stage;

    return Result::kNoResource;
}
void MessageParser::reset() {
    _stage = MessageParseStage::kInit;
}
void MessageParser::_checkLengthSchema(const MessageLengthSchema* lengthSchema,
                                       bool                       isDefault) const {
    ASSERT(isDefault || (_schema.commandSize != DataWidth::kNone),
           "command size must be none, if use multiple length definition.");

    switch (lengthSchema->mode) {
        case MessageLengthSchemaMode::kFixedLength:
            // ASSERT(_schema.prefixSize != 0, "fixed mode: prefix size must not be 0.");

            break;
        case MessageLengthSchemaMode::kDynamicLength:
            ASSERT(_schema.prefixSize != 0, "dynamic mode: prefix size must not be 0.");
            ASSERT(lengthSchema->dynamic.lengthSize != DataWidth::kNone,
                   "dynamic mode: length size must not be 0.");

            break;
        case MessageLengthSchemaMode::kFreeLength:
            ASSERT(_schema.suffixSize != 0, "free mode: suffix size must not be 0.");
            ASSERT(_schema.crcSize == DataWidth::kNone, "free mode: crc not supported.");
            break;
        default:
            break;
    }
};

void MessageParser::_checkSchema() const {
    ASSERT(static_cast<u8>(_schema.commandSize) <= kMessageParserCmdLengthCrcBufferSize,
           "cmd length must not less than %lu.", kMessageParserCmdLengthCrcBufferSize);
    ASSERT(static_cast<u8>(_schema.crcSize) <= kMessageParserCmdLengthCrcBufferSize,
           "crc length must not less than %lu.", kMessageParserCmdLengthCrcBufferSize);

    for (u32 i = 0; i < _schema.lengthSchemaCount; ++i) {
        auto& def = _schema.lengthSchemas[i];
        _checkLengthSchema(&def.length, false);
    }
    _checkLengthSchema(&_schema.defaultLength, true);
}

bool MessageParser::_seek(const u8 (&pattern)[kMessageSchemaPrefixSuffixMaxSize], u8 patternSize) {
    u32  totalLength = _buffer.getSize();
    auto offset      = _offset;
    while ((offset + patternSize) <= totalLength) {
        auto matched = true;
        for (u8 i = 0; i < patternSize; ++i) {
            if (pattern[i] != *_buffer.peekPtr(offset + i)) {
                matched = false;
                break;
            }
        }
        if (matched) {
            _offset = offset;
            return true;
        }
        offset++;
    }
    _offset = offset;
    return false;
}
i32 MessageParser::_match(const u8 (&pattern)[kMessageSchemaPrefixSuffixMaxSize], u8 patternSize) {
    u32 totalLength = _buffer.getSize();
    if (_offset + patternSize > totalLength) {
        return -1;
    }
    for (u8 i = 0; i < patternSize; ++i) {
        if (pattern[i] != *_buffer.peekPtr(_offset + i)) {
            return 0;
        }
    }
    _offset += patternSize;
    return 1;
}
bool MessageParser::_fetch(u8* data, u16 length) {
    if (_offset + length > _buffer.getSize()) {
        return false;
    }
    _buffer.peek(data, _offset, length);
    _offset += length;
    return true;
}
bool MessageParser::_move(u16 length) {
    if (_offset + length > _buffer.getSize()) {
        return false;
    }
    _offset += length;
    return true;
}
bool MessageParser::_remove(u16 length) {
    if (length > _buffer.getSize()) {
        return false;
    }
    _buffer.readVirtual(length);
    _offset -= length;
    return true;
}
const MessageLengthSchema* MessageParser::_lengthSchemaMatch() {
    for (u32 i = 0; i < _schema.lengthSchemaCount; ++i) {
        auto& def        = _schema.lengthSchemas[i];
        auto  cmdMatched = true;
        for (int j = 0; j < static_cast<u8>(_schema.commandSize); ++j) {
            if (def.command[j] != _command[j]) {
                cmdMatched = false;
                break;
            }
        }
        if (cmdMatched) {
            return &def.length;
        }
    }
    return &_schema.defaultLength;
}
u32 MessageParser::_parseLength(const MessageLengthSchema* lengthSchema,
                                u8 (&buf)[kMessageParserCmdLengthCrcBufferSize]) const {
    if (lengthSchema->dynamic.lengthSize == DataWidth::k8Bits) {
        return buf[0];
    } else if (lengthSchema->dynamic.lengthSize == DataWidth::k16Bits) {
        return arch::getUint16(static_cast<u8*>(buf), lengthSchema->dynamic.endian);
    } else if (lengthSchema->dynamic.lengthSize == DataWidth::k32Bits) {
        return arch::getUint32(buf, lengthSchema->dynamic.endian);
    } else {
        return 0;
    }
}
void MessageParser::_prepareFrame() {
    _freeContentStartIndex = 0;
    _contentLength         = 0;
    for (u8 i = 0; i < kMessageParserCmdLengthCrcBufferSize; i++) {
        _command[i] = 0;
    }
}
}  // namespace wibot
