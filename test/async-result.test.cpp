#include "../src/os/async-result.hpp"
#include <utility> // for std::move

namespace wibot {

class AsyncResultTest {
public:
  static void testBasicUsage() {
    // 创建异步源
    AsyncSource source = AsyncSource();

    // 获取异步结果
    AsyncResult result = source.getResult();

    // 测试超时等待
    Result waitResult = result.wait(100); // 100ms timeout
    // 应该超时

    // 设置完成
    source.setDone();

    // 现在应该立即返回成功
    waitResult = result.wait(100);
    // 应该成功
  }

  static void testErrorHandling() {
    AsyncSource source = AsyncSource();
    AsyncResult result = source.getResult();

    // 设置错误
    source.setError(Result::kError);

    // 等待应该返回错误
    Result waitResult = result.wait(100);
    (void)waitResult; // 应该返回错误
  }

  static void testMoveSemantics() {
    AsyncSource source = AsyncSource();
    AsyncResult result1 = source.getResult();

    // 移动构造
    AsyncResult result2 = std::move(result1);

    // 设置完成
    source.setDone();

    // result2 应该能正常工作
    Result waitResult = result2.wait(100);
    (void)waitResult; // 应该成功
  }
};

// 示例用法（类似用户提供的示例）
class StreamReader {
public:
  StreamReader() : _source(AsyncSource()) {}

  Result begin() { return Result::kOk; }

  Result end() { return Result::kOk; }

  AsyncResult read(void *data) {
    // 模拟开始异步读取
    return _source.getResult();
  }

  void _isr_complete() {
    // 模拟中断完成
    _source.setDone();
  }

  void _isr_error() {
    // 模拟中断错误
    _source.setError(Result::kError);
  }

private:
  AsyncSource _source;
};

class StreamWorker {
public:
  void run() {
    Result rst = _stream.begin();
    if (!rst.isOk()) {
      // error handler
      return;
    }

    // 模拟读取数据
    char frame[256];
    AsyncResult ar = _stream.read(&frame);

    // 模拟异步完成
    _stream._isr_complete();

    rst = ar.wait(1000);
    if (rst.isOk()) {
      // do post handler
    } else {
      // handle error or timeout
    }
  }

private:
  StreamReader _stream;
};

void runAllTests() {
  AsyncResultTest::testBasicUsage();
  AsyncResultTest::testErrorHandling();
  AsyncResultTest::testMoveSemantics();

  // 测试示例用法
  StreamWorker worker;
  worker.run();
}

} // namespace wibot