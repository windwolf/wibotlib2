//
// Created by zhouj on 2023/9/13.
//
#include "type.hpp"

namespace wibot {

Result::Result() : value(0) {
}

Result::Result(ResultStatus status) : value(static_cast<u32>(status)) {
}

Result::Result(ResultStatus status, u32 errorCode)
    : value((errorCode << 8) | static_cast<u32>(status)) {
}

Result::Result(u32 halStatus) : value(halStatus) {
}

bool Result::operator==(const Result &other) const {
    return this->value == other.value;
}

bool Result::operator!=(const Result &other) const {
    return this->value != other.value;
}

bool Result::isOk() const {
    return (value & 0xFF) == static_cast<u32>(ResultStatus::kOk);
}

bool Result::isError() const {
    return (value & 0xFF) == static_cast<u32>(ResultStatus::kError);
}

bool Result::isBusy() const {
    return (value & 0xFF) == static_cast<u32>(ResultStatus::kBusy);
}

bool Result::isTimeout() const {
    return (value & 0xFF) == static_cast<u32>(ResultStatus::kTimeout);
}

u32 Result::getErrorCode() const {
    return (value >> 8);
}

const Result Result::kOk(ResultStatus::kOk);
const Result Result::kError(ResultStatus::kError);
const Result Result::kBusy(ResultStatus::kBusy);
const Result Result::kTimeout(ResultStatus::kTimeout);
const Result Result::kInvalidParameter(ResultStatus::kInvalidParameter);  // 静态常量成员
const Result Result::kNoResource(ResultStatus::kNoResource);              // 静态常量成员
const Result Result::kNotSupport(ResultStatus::kNotSupport);              // 静态常量成员

u8 Codex::byteToBcd(u8 Value) {
    u32 bcdhigh = 0U;
    u8  Param   = Value;

    while (Param >= 10U) {
        bcdhigh++;
        Param -= 10U;
    }

    return ((u8)(bcdhigh << 4U) | Param);
};
u8 Codex::bcdToByte(u8 Value) {
    u32 tmp;
    tmp = (((u32)Value & 0xF0U) >> 4U) * 10U;
    return (u8)(tmp + ((u32)Value & 0x0FU));
};

LinkList::LinkList() {
    _next = nullptr;
};

LinkList *LinkList::append(LinkList *node) {
    LinkList *p = this;
    while ((p->_next != nullptr) && (p != node)) {
        p = p->_next;
    }
    if (p == node) {
        return nullptr;
    }
    p->_next    = node;
    node->_next = nullptr;
    return node;
};

LinkList *LinkList::remove(LinkList *node) {
    LinkList *p = this;

    while ((p->_next != nullptr) && (p->_next != node)) {
        p = p->_next;
    }

    if (p->_next == node) {
        p->_next    = node->_next;
        node->_next = nullptr;
        return node;
    }
    return nullptr;
};

char *String::trim(char *str, char delimit) const {
    while (*str != 0x00) {
        if (*str == delimit) {
            str++;
            continue;
        } else {
            return str;
        }
    }

    return str;
};
}  // namespace wibot
