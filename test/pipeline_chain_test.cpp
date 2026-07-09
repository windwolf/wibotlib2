#include <cassert>

#include "pp/pipeline.hpp"
#include "pp/source/constant-source.hpp"

using namespace wibot;

namespace {

class GainNode : public INode {
   public:
    struct Inputs {
        In<f32> x;
    } inputs;

    struct Outputs {
        Out<f32> y;
        Out<u32> processCount;
    } outputs;

    explicit GainNode(f32 gain) : _gain(gain) {
    }

    bool ready() override {
        return inputs.x.bound();
    }

    void process() override {
        if (!outputs.y.bound() && !outputs.processCount.bound()) {
            return;
        }
        if (outputs.y.bound()) {
            outputs.y.ref() = inputs.x.get() * _gain;
        }
        if (outputs.processCount.bound()) {
            outputs.processCount.ref() = ++_processCount;
        }
    }

    void reset() override {
        _processCount = 0;
    }

   private:
    f32 _gain;
    u32 _processCount{0};
};

class OptionalInputNode : public INode {
   public:
    struct Inputs {
        In<f32> x;
    } inputs;

    bool ready() override {
        return true;
    }

    void process() override {
    }

    void reset() override {
    }
};

}  // namespace

// 不依赖 RTOS，验证 Pipeline 构建、可选输出和错误传播。
void pipeline_chain_compile_test() {
    {
        PipelineChainBuilder<3> builder;

        f32 sourceValue = 0.0f;
        f32 result      = 0.0f;

        ConstantSourceNode<f32> source(1.5f);
        ConstantSourceNode<f32> unusedSource(8.0f);
        GainNode                gain(2.0f);

        builder.bind(source.outputs.x, sourceValue);
        builder.bind(gain.outputs.y, result);
        // unusedSource.x 和 gain.processCount 均未绑定，构建和执行仍应成功。

        builder.addNode(source);
        builder.addNode(unusedSource);
        builder.addNode(gain);

        bool connected = builder.connect(source, source.outputs.x, gain, gain.inputs.x);
        assert(connected);

        PipelineChain<3> chain;
        assert(builder.build(chain));

        chain.reset();
        chain.tick();
        assert(result == 3.0f);
    }

    {
        PipelineChainBuilder<2> builder;

        ConstantSourceNode<f32> source(1.0f);
        OptionalInputNode       sink;

        builder.addNode(source);
        builder.addNode(sink);

        // 连接使用了未绑定存储的 Output：connect 失败且错误会传播到 build。
        assert(!builder.connect(source, source.outputs.x, sink, sink.inputs.x));

        PipelineChain<2> chain;
        assert(!builder.build(chain));
    }
}

extern "C" void pipeline_chain_test_entry() {
    pipeline_chain_compile_test();
}
