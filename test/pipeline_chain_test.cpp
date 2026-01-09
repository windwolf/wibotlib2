#include <cassert>

#include "nodes_examples.hpp"
#include "pipeline.hpp"

using namespace wibot;

// 仅编译期/简单运行期验证：不依赖RTOS，作为构建检查。
void pipeline_chain_compile_test() {
  PipelineChainBuilder<8> builder;

  // 外部存储（独立于节点）
  float sigA = 0.0f;
  float sigB = 0.0f;
  float sigSum = 0.0f;
  float sigLpf = 0.0f;

  // 节点及其外部状态/配置
  ConstSourceNode::Storage srcAStore{1.0f};
  ConstSourceNode srcA(srcAStore);
  ConstSourceNode::Storage srcBStore{2.0f};
  ConstSourceNode srcB(srcBStore);
  Sum2Node sum;
  LowpassNode::Config lcfg;
  lcfg.sampleTime = 0.001f;
  lcfg.cutoffFreq = 50.0f;
  lcfg.wrapValue = 0.0f;
  LowpassNode::Storage lst;
  LowpassNode lpf(lcfg, lst);

  // 绑定输出端口到外部存储
  PipelineChainBuilder<8>::bind(srcA.outputs.x, sigA);
  PipelineChainBuilder<8>::bind(srcB.outputs.x, sigB);
  PipelineChainBuilder<8>::bind(sum.outputs.y, sigSum);
  PipelineChainBuilder<8>::bind(lpf.outputs.y, sigLpf);

  // 连接输入端口到上游输出（多播可重复 connect）
  const auto idSrcA = builder.addNode(srcA);
  const auto idSrcB = builder.addNode(srcB);
  const auto idSum = builder.addNode(sum);
  const auto idLpf = builder.addNode(lpf);

  bool ok = true;
  ok &= builder.connect(idSrcA, srcA.outputs.x, idSum, sum.inputs.a);
  ok &= builder.connect(idSrcB, srcB.outputs.x, idSum, sum.inputs.b);
  ok &= builder.connect(idSum, sum.outputs.y, idLpf, lpf.inputs.x);
  assert(ok);

  // 自动拓扑排序并构建运行时 chain（builder._edges 此后可释放）
  PipelineChain<8> chain;
  ok = builder.build(chain);
  assert(ok);

  // 执行若干拍
  chain.reset();
  chain.tick();
  chain.tick();

  const float y = sigLpf;
  (void)y;
  // 结果应接近 (1+2)=3 经过低通处理后的数值，范围大致在[1,3]之间逐步收敛
  assert(y > 0.0f);
}

// 若 ThreadX 测试框架需要入口，这里仅提供空壳调用。
extern "C" void pipeline_chain_test_entry() { pipeline_chain_compile_test(); }
