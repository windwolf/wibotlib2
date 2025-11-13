
#include "circular-buffer.hpp"
#include "minunit.hpp"
#include "string.h"

namespace wibot::test {
LOGGER("mp test");
static const uint8_t refData[8] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};
static void          message_parser_fixed_test_1() {
    LOG_D("-----message_parser_fixed_test_1----------");
    uint8_t buf[64] = {0};

    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    static MessageSchema schema = {
                 .prefix     = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD},
                 .prefixSize = 7,
                 .defaultLength{
                     .mode = MessageLengthSchemaMode::kFixedLength,
                     .fixed{
                         .length = 8,
            },
        },
                 .crcSize    = DataWidth::kNone,
                 .suffix     = {0x0E, 0x0F},
                 .suffixSize = 2,

    };
    MessageParser parser = MessageParser(schema, rb);

    uint8_t wr0Data[3]  = {0x33, 0xFA, 0xFB};
    uint8_t wr1Data[17] = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD, 0x01, 0x01,
                                    0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F};
    uint8_t wr2Data[17] = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD, 0x01, 0x01,
                                    0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};
    uint8_t wr3Data[13] = {0x00, 0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01,
                                    0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};

    rb.write(wr0Data, 3, true);
    rb.write(wr1Data, 17, true);
    rb.write(wr2Data, 17, true);
    rb.write(wr3Data, 13, true);

    MessageFrame frame(rstBuf);
    Result       rst;

    // test1_1:2
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void message_parser_fixed_test_2() {
    LOG_D("-----message_parser_fixed_test_2----------");
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageSchema schema = {
        .prefix     = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD},
        .prefixSize = 7,
        .defaultLength =
            {
                .mode = MessageLengthSchemaMode::kFixedLength,
                .fixed =
                    {
                        .length = 8,
                    },
            },
        .crcSize    = DataWidth::kNone,
        .suffixSize = 0,
    };
    MessageParser parser(std::move(schema), rb);

    uint8_t wr0Data[3]  = {0x33, 0xFA, 0xFB};
    uint8_t wr1Data[15] = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD, 0x01,
                           0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};
    uint8_t wr2Data[15] = {0xFA, 0xFB, 0xFC, 0xFD, 0xFA, 0xFB, 0xFD, 0x01,
                           0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};
    uint8_t wr3Data[11] = {0x00, 0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};

    rb.write(wr0Data, 3, true);
    rb.write(wr1Data, 15, true);
    rb.write(wr2Data, 15, true);
    rb.write(wr3Data, 11, true);

    MessageFrame frame(rstBuf);
    Result       rst;
    // test1_2:1
    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void message_parser_multi_schema_test_1() {
    LOG_D("-----message_parser_multi_schema_test_1----------");
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[12] = {0};
    Slice                   rstBuf   = Slice(buf2, 12);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageLengthSchemaDefinition defs[2]{
        {
            .command{0x01},
            .length{
                .mode = MessageLengthSchemaMode::kFixedLength,
                .fixed{.length = 1},
            },
        },
        {
            .command{0x02},
            .length{
                .mode = MessageLengthSchemaMode::kFixedLength,
                .dynamic{.lengthSize = DataWidth::k8Bits, .endian = Endian::kBig},
            },
        },
    };
    MessageSchema schema = {
        .prefix            = {0xFA, 0xFB},
        .prefixSize        = 2,
        .commandSize       = DataWidth::k8Bits,
        .lengthSchemas     = defs,
        .lengthSchemaCount = 2,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kFixedLength,
            .fixed{
                .length = 0,
            },
        },
        .alterDataSize = DataWidth::k8Bits,
        .crcSize       = DataWidth::k8Bits,
        .suffix        = {0xF0, 0xF1},
        .suffixSize    = 2,
    };
    MessageParser parser(std::move(schema), rb);

    uint8_t wr0Data[3]  = {0x33, 0xFA, 0xFB};
    uint8_t wr1Data[15] = {0xFA, 0xFB, 0x01, 0x0F, 0x10, 0xCC, 0xF0, 0xF1,
                           0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};
    uint8_t wr2Data[15] = {0xFA, 0xFB, 0x02, 0x02, 0x0F, 0x10, 0x11, 0xCC,
                           0xF0, 0xF1, 0x01, 0x01, 0x01, 0x01, 0x01};
    uint8_t wr3Data[11] = {0xFA, 0xFB, 0x03, 0x0F, 0xCC, 0xF0, 0xF1, 0x01, 0x02, 0x03, 0x04};

    rb.write(wr0Data, 3, true);
    rb.write(wr1Data, 15, true);
    rb.write(wr2Data, 15, true);
    rb.write(wr3Data, 11, true);

    MessageFrame frame(rstBuf);
    Result       rst;
    uint8_t      alt_ref[1] = {0x0F};
    uint8_t      crc_ref[1] = {0xCC};

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x01};
        uint8_t ctn[1] = {0x10};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto content = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(content, ctn, 1);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x02};
        uint8_t ctn[2] = {0x10, 0x11};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto content = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(content, ctn, 2);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x03};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }

    rb.write(wr0Data, 3, true);
    rb.write(wr1Data, 15, true);
    rb.write(wr2Data, 15, true);
    rb.write(wr3Data, 11, true);

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x01};
        uint8_t ctn[1] = {0x10};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto content = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(content, ctn, 1);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x02};
        uint8_t ctn[2] = {0x10, 0x11};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto content = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(content, ctn, 2);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }

    rst = parser.parse(&frame);  // 1
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        uint8_t cmd[1] = {0x03};
        auto    prefix = frame.getPrefix().data;
        MU_ASSERT_VEC_EQUALS(prefix, schema.prefix, 2);
        auto command = frame.getCommand().data;
        MU_ASSERT_VEC_EQUALS(command, cmd, 1);
        auto alterData = frame.getAlterdata().data;
        MU_ASSERT_VEC_EQUALS(alterData, alt_ref, 1);
        auto crc = frame.getCrc().data;
        MU_ASSERT_VEC_EQUALS(crc, crc_ref, 1);
        auto suffix = frame.getSuffix().data;
        MU_ASSERT_VEC_EQUALS(suffix, schema.suffix, 2);
    }
}

static void message_parser_dynamic_test_1() {
    LOG_D("-----message_parser_dynamic_test_1----------");
    MessageSchema schema = {

        .prefix     = {0xEF, 0xFF},
        .prefixSize = 2,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kDynamicLength,
            .dynamic{
                .lengthSize = DataWidth::k8Bits,
                .range      = kMessageSchemaRangeContent,
            },
        },
        .crcSize    = DataWidth::kNone,
        .suffix     = {0x0E, 0x0F},
        .suffixSize = 2,
    };
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(schema, rb);

    uint8_t wr0Data[43] = {
        0x33, 0xFA, 0xFB,                                                                     // 3
        0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F,         // 13
        0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F,         // 13
        0x00, 0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};  // 14

    rb.write(wr0Data, 43, true);

    MessageFrame frame(rstBuf);
    Result       rst;
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void message_parser_dynamic_test_2() {
    LOG_D("-----message_parser_dynamic_test_2----------");
    MessageSchema schema = {

        .prefix     = {0xEF, 0xFF},
        .prefixSize = 2,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kDynamicLength,
            .dynamic{
                .lengthSize = DataWidth::k8Bits,
                .range      = kMessageSchemaRangeContent,
            },
        },
        .crcSize    = DataWidth::kNone,
        .suffixSize = 0,
    };
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(std::move(schema), rb);

    uint8_t wr0Data[43] = {
        0x33, 0xFA, 0xFB,                                                                     // 3
        0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F,         // 13
        0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F,         // 13
        0x00, 0xEF, 0xFF, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};  // 14

    rb.write(wr0Data, sizeof(wr0Data), true);

    MessageFrame frame(rstBuf);
    Result       rst;
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void message_parser_dynamic_test_3() {
    LOG_D("-----message_parser_dynamic_test_3----------");
    MessageSchema schema = {

        .prefix      = {0xB5, 0x62},
        .prefixSize  = 2,
        .commandSize = DataWidth::k16Bits,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kDynamicLength,
            .dynamic{
                .lengthSize = DataWidth::k16Bits,
                .range      = kMessageSchemaRangePrefix | kMessageSchemaRangeCmd |
                         kMessageSchemaRangeLength | kMessageSchemaRangeContent |
                         kMessageSchemaRangeCrc,
            },
        },
        .crcSize    = DataWidth::kNone,
        .suffixSize = 0,
    };
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(schema, rb);

    uint8_t wr0Data[50] = {0x33,  // 1
                           0xB5, 0x62, 0x01, 0x02, 0x0F, 0x00, 0x01, 0x01,
                           0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F,  // 16
                           0x33,                                            // 1
                           0xB5, 0x62, 0x01, 0x02, 0x0F, 0x00, 0x01, 0x01,
                           0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F,  // 16
                           0xB5, 0x62, 0x01, 0x02, 0x0F, 0x00, 0x01, 0x01,
                           0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};  // 16

    rb.write(wr0Data, 50, true);

    MessageFrame frame(rstBuf);
    Result       rst;
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rb.write(wr0Data, 50, true);

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void free_mode_test_1() {
    LOG_D("-----free_mode_test_1----------");
    MessageSchema schema = {

        .prefix     = {0xEF, 0xFF},
        .prefixSize = 2,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kFreeLength,
        },
        .crcSize    = DataWidth::kNone,
        .suffix     = {0x0E, 0x0F},
        .suffixSize = 2,
    };
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(schema, rb);

    uint8_t wr0Data[40] = {
        0x33, 0xFA, 0xFB,                                                               // 3
        0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F,         // 12
        0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F,         // 12
        0x00, 0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x0E, 0x0F};  // 13

    rb.write(wr0Data, sizeof(wr0Data), true);

    MessageFrame frame(rstBuf);
    Result       rst;
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        static const uint8_t refData2[21] = {
            0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x1E, 0x0F,  // 12
            0x00, 0xEF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04};
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData2, 21);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst != Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 8);
    }
}

static void free_mode_test_2() {
    LOG_D("-----free_mode_test_2----------");
    MessageSchema schema = {
        //.prefix = {0xEF, 0xFF},
        .prefixSize = 0,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kFreeLength,
        },
        .crcSize    = DataWidth::kNone,
        .suffix     = {'\r', '\n'},
        .suffixSize = 2,

    };
    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(schema, rb);

    const char *wr0Data = "hello, message parser.\r\nhello, ";

    rb.write(castPointer<uint8_t>(const_cast<char *>(wr0Data)), strlen(wr0Data), true);

    MessageFrame frame(rstBuf);
    Result       rst;

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto ctn = frame.getContent();
        MU_ASSERT(memcmp(ctn.data, wr0Data, ctn.size) == 0);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kNoResource);

    const char *wr1Data = "free_mode_test_2\r\nhello, i just wanna you sack!\r";
    rb.write(castPointer<uint8_t>(const_cast<char *>(wr1Data)), strlen(wr1Data), true);

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto ctn = frame.getContent();
        MU_ASSERT(memcmp(ctn.data, "hello, free_mode_test_2", ctn.size) == 0);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst != Result::kOk);
}

static void static_mode_test_1() {
    LOG_D("-----static_mode_test_1----------");
    static MessageSchema schema = {

        .prefix     = {0xFF, 0xFE},
        .prefixSize = 2,

        .commandSize = DataWidth::kNone,
        .defaultLength{
            .mode = MessageLengthSchemaMode::kFixedLength,
            .fixed{
                .length = 14,
            },
        },

        .alterDataSize = DataWidth::kNone,
        .crcSize       = DataWidth::kNone,

        .suffixSize = 0,
    };

    uint8_t                 buf[64]  = {0};
    uint8_t                 buf2[64] = {0};
    Slice                   rstBuf   = Slice(buf2, 64);
    CircularBuffer<uint8_t> rb(buf, 64);

    MessageParser parser(schema, rb);

    uint8_t wr0Data[16] = {0xFF, 0xFE, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    uint8_t refData[14] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

    rb.write(wr0Data, sizeof(wr0Data), true);

    MessageFrame frame(rstBuf);
    Result       rst;
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 14);
    }

    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kNoResource);

    rb.write(wr0Data, sizeof(wr0Data), true);
    rst = parser.parse(&frame);
    MU_ASSERT(rst == Result::kOk);
    if (rst == Result::kOk) {
        auto fdata = frame.getContent().data;
        MU_ASSERT_VEC_EQUALS(fdata, refData, 14);
    }
}  // namespace wibot::comm::test

void messageParserTest() {
    LOG_D("-----messageParserTest listen----------");
    MU_ASSERT(sizeof(float) == 4);
    MU_ASSERT(sizeof(double) == 8);
    message_parser_fixed_test_1();
    message_parser_fixed_test_2();
    message_parser_dynamic_test_1();
    message_parser_dynamic_test_2();
    message_parser_dynamic_test_3();
    free_mode_test_1();
    free_mode_test_2();
    static_mode_test_1();
    message_parser_multi_schema_test_1();
    LOG_D("-----messageParserTest finish----------");
}
}  // namespace wibot::test
