#pragma once

#include "type.hpp"
#include <functional>
#include <concepts>

namespace wibot::dsp {
template <typename TIn, typename TOut>
using MappingFunction = std::function<TOut(TIn)>;
template <typename TIn, typename TOut>
class CustomMapper {
   public:
    struct Config {
        MappingFunction<TIn, TOut> mappingFunc{};
    };

   public:
    explicit CustomMapper(Config& config) : _config(config) {
        if (!_config.mappingFunc) {
            // 默认映射：直接转换类型
            _config.mappingFunc = [](TIn input) -> TOut { return static_cast<TOut>(input); };
        }
    }

    TOut map(TIn input) {
        return _config.mappingFunc(input);
    }

    void setConfig(Config& config) {
        if (config.mappingFunc) {
            _config = config;
        }
    }

    bool isConfigValid() const {
        return static_cast<bool>(_config.mappingFunc);
    }

   private:
    Config& _config{};
};

}  // namespace wibot::dsp
