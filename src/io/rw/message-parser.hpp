#pragma once

#include "type.hpp"
#include "buffer.hpp"
#include "circular-buffer.hpp"

namespace wibot {
constexpr u32 kMessageParserCmdLengthCrcBufferSize = 4;
constexpr u32 kMessageSchemaPrefixSuffixMaxSize    = 8;

enum class MessageParseStage : u8 {
    kInit = 0,          // schema is changed, reset everything, reparse current buffer.
    kPreparing,         // Prepare to parse a new message.
    kSeekingPrefix,     // try to seek the message's begin flags.
    kParsingCmd,        // try to parse cmd of the message.
    kParsingLength,     // try to parse the length of the message.
    kParsingAlterdata,  // try to parse the alterdata of the message.
    kSeekingContent,    // try to seek the content of the message.
    kSeekingCrc,        // try to seek the crc of the message.
    kMatchingSuffix,    // try to match the suffix of the message. if matched, the message is parsed
                        // successfully.
    kDone,
};
enum class MessageLengthSchemaMode : u8 {
    kFixedLength = 0,
    kDynamicLength,
    kFreeLength,
};

using MessageSchemaRange = u8;

constexpr MessageSchemaRange kMessageSchemaRangePrefix    = 0x01;
constexpr MessageSchemaRange kMessageSchemaRangeCmd       = 0x02;
constexpr MessageSchemaRange kMessageSchemaRangeLength    = 0x04;
constexpr MessageSchemaRange kMessageSchemaRangeAlterdata = 0x08;
constexpr MessageSchemaRange kMessageSchemaRangeContent   = 0x10;
constexpr MessageSchemaRange kMessageSchemaRangeCrc       = 0x20;
constexpr MessageSchemaRange kMessageSchemaRangeSuffix    = 0x40;
constexpr MessageSchemaRange kMessageSchemaRangeAll       = 0x7F;

enum MessageSchemaCrcMode {
    k8BitFletcher,
};

struct MessageLengthSchema {
    MessageLengthSchemaMode mode;
    union {
        struct {
            /**
             * @brief The length of (content) in bytes.
             * @note Must not be 0.
             */
            u32 length;

        } fixed;
        struct {
            DataWidth          lengthSize;  // the size of the length field.
            Endian             endian;
            MessageSchemaRange range;
        } dynamic;
    };
};

struct MessageLengthSchemaDefinition {
    u8                  command[kMessageParserCmdLengthCrcBufferSize];
    MessageLengthSchema length;
};

/**
 * @brief
 * fixed   :
 * |  prefix  | (cmd)          | (alterData) | (content) | (crc) | (suffix) |
 * dynamic :
 * |  prefix  | (cmd) | length | (alterData) | (content) | (crc) | (suffix) |
 * free    :
 * | (prefix) | (cmd)          | (alterData) | (content)         |  suffix  |
 */
struct MessageSchema {
    u8 prefix[kMessageSchemaPrefixSuffixMaxSize];
    u8 prefixSize;  // prefix size. 1-8, prefix size must not be 0, except

    DataWidth commandSize;  // cmd size. 0-4.

    /**
     * multi length definitions witch match the command.
     */
    MessageLengthSchemaDefinition* lengthSchemas;
    u32                            lengthSchemaCount;
    MessageLengthSchema            defaultLength;

    DataWidth alterDataSize;

    DataWidth          crcSize;
    MessageSchemaRange crcRange;
    u8                 suffix[kMessageSchemaPrefixSuffixMaxSize];
    u8                 suffixSize;  // suffix size. 0-8, 0 meaning that suffix is not
    // present. if mode = free, this field must not be 0.

    const MessageLengthSchema& getLengthSchema(const u8* command) const;

    u32 getContentOverhead(const MessageLengthSchema* lengthSchema) const;

    u32 getDynamicLengthOverhead(const MessageLengthSchema* lengthSchema) const;

    /**
     * @brief get the length of the content.
     * @param contentLength The length of the content. @note contentLength is ignored if the mode is
     * fixed
     * @return
     */
    u32 getLength(const MessageLengthSchema* lengthSchema, u32 contentLength) const;
};
class MessageParser;

struct MessageFrameSegment {
    u16 offset;
    u16 length;
};
struct MessageFrame {
   public:
    MessageFrame(Slice buffer);
    /**
     * @brief Construct a new Message Frame object, used for sending.
     * @param buffer
     * @param schema
     * @param lengthSchema
     * @param contentLength
     */
    MessageFrame(Slice buffer, const MessageSchema& schema, u8* command, u32 contentLength = 0);

    Result ensureSchema(const MessageSchema& schema);

    Slice getPrefix() const;
    Slice getCommand() const;
    Slice getLength() const;
    Slice getAlterdata() const;
    Slice getContent() const;
    Slice getCrc() const;
    Slice getSuffix() const;
    const Slice& getWholeBuffer() const;
    Slice buildFrame(const MessageSchema& schema);

   private:
    friend class MessageParser;
    MessageFrameSegment _prefix;
    MessageFrameSegment _command;
    MessageFrameSegment _length;
    MessageFrameSegment _alterData;
    MessageFrameSegment _content;
    MessageFrameSegment _crc;
    MessageFrameSegment _suffix;
    Slice               _buffer;
    u32                 _frameLength;
};

class MessageParser {
   public:
    explicit MessageParser(const MessageSchema& schema, CircularBuffer8& buffer);

    /**
 * @brief Parse incoming data to extract a message frame according to the schema.
 * 
 * This method processes the data buffer and attempts to extract a valid message frame
 * based on the configured message schema. The parsing process follows a state machine
 * approach, going through various stages from initialization to completion.
 * 
 * The parsing stages include:
 * - Initialization
 * - Preparation
 * - Seeking prefix
 * - Parsing command
 * - Parsing length
 * - Parsing alternate data
 * - Seeking content
 * - Seeking CRC
 * - Matching suffix
 * - Completion
 * 
 * @param parsedFrame Pointer to a MessageFrame object that will hold the parsed message.
 *                    This parameter cannot be nullptr.
 * @param interFrameGap If true, adjusts the buffer to ensure there's just enough data
 *                      for a complete frame, discarding excess data. Default is false.
 *                  This is useful for handling inter-frame gaps in a stream of messages.
 * 
 * @return Result::kOk if a complete message frame was successfully parsed.
 *         Result::kInvalidParameter if the parsedFrame parameter is nullptr.
 *         Result::kNoResource if there is not enough data in the buffer to complete parsing.
 * 
 * @note This method maintains state between calls, allowing incremental parsing as data
 *       becomes available. If a different frame object is provided than in a previous call,
 *       the parser will reset to the initialization stage.
 * @warning interFrameGap can only be used in fixed length mode. 
 * In the interFrameGap mode, the data will lose if frame length is not fixed, because the HALF COMPLETED event will break the frame.
 */
    Result parse(MessageFrame* parsedFrame, bool interFrameGap = false);
    void   reset();

   private:
    const MessageSchema&       _schema;
    CircularBuffer8&           _buffer;
    MessageParseStage          _stage;
    u32                        _offset;  // current working seek offset. initial value is -1.
    u32                        _freeContentStartIndex;
    const MessageLengthSchema* _lengthSchema;
    u32                        _contentLength;
    u32                        _contentOverhead;
    MessageFrame*              _frame;
    u8                         _command[kMessageParserCmdLengthCrcBufferSize];

    void _checkSchema() const;
    void _checkLengthSchema(const MessageLengthSchema* lengthSchema, bool isDefault) const;

    /**
     * seek the pattern in the buffer, from the current offset to the end.
     * if found, the offset will be set to the beginning of the pattern.
     * otherwise the offset will set to seek position.
     * @param pattern
     * @param patternSize
     * @return Return true if the pattern is found, otherwise false.
     */
    bool _seek(const u8 (&pattern)[kMessageSchemaPrefixSuffixMaxSize], u8 patternSize);
    ;

    /**
     * match the pattern in the buffer, at the current offset. if matched, the
     * offset will be set to the the next position of the pattern. otherwise the
     * offset will not be changed.
     * @param pattern
     * @param patternSize
     * @return If matched, return 1, not matched, return 0, no enough space
     * return -1.
     */
    i32 _match(const u8 (&pattern)[kMessageSchemaPrefixSuffixMaxSize], u8 patternSize);

    /**
     * @brief fetch data from the buffer, at the current offset, and move the
     * offset to the next position.
     * @param data
     * @param length
     * @return Return true if the data is fetched successfully, otherwise false.
     */
    bool _fetch(u8* data, u16 length);
    /**
     * @brief move the offset to the next position.
     * @param length
     * @return Return true if the offset is moved successfully, otherwise false.
     */
    bool _move(u16 length);

    /**
     * @brief remove data from the buffer, at the current offset, and sync the
     * offset.
     * @param length
     * @return Return true if the data is removed successfully, otherwise false.
     */
    bool _remove(u16 length);

    const MessageLengthSchema* _lengthSchemaMatch();

    u32 _parseLength(const MessageLengthSchema* lengthSchema,
                     u8 (&buf)[kMessageParserCmdLengthCrcBufferSize]) const;

    void _prepareFrame();
};

}  // namespace wibot
