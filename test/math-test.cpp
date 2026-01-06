//
// Math library test suite
//

#include "minunit.hpp"
#include "operator.hpp"
#include <cmath>


LOGGER("math.test")

namespace wibot::test {

// Test fixtures
static Math math;

// ============ Basic Arithmetic Tests ============

static void testAddQ15() {
  q15 a = 1000;
  q15 b = 2000;
  q15 result = math.add<q15>(a, b);
  MU_ASSERT_EQUALS(result, 3000);

  // Test saturation
  q15 max_val = 32767;
  result = math.add<q15>(max_val, 1000);
  MU_ASSERT_EQUALS(result, 32767); // Saturated at max
}

static void testSubQ15() {
  q15 a = 5000;
  q15 b = 2000;
  q15 result = math.sub<q15>(a, b);
  MU_ASSERT_EQUALS(result, 3000);

  // Test saturation
  q15 min_val = -32768;
  result = math.sub<q15>(min_val, 1000);
  MU_ASSERT_EQUALS(result, -32768); // Saturated at min
}

static void testMulQ15() {
  q15 a = 16384; // 0.5 in Q15
  q15 b = 16384; // 0.5 in Q15
  q15 result = math.mul<q15>(a, b);
  // 0.5 * 0.5 = 0.25 = 8192 in Q15
  MU_ASSERT_EQUALS(result, 8192);
}

static void testAddQ31() {
  q31 a = 1000000;
  q31 b = 2000000;
  q31 result = math.add<q31>(a, b);
  MU_ASSERT_EQUALS(result, 3000000);
}

static void testSubQ31() {
  q31 a = 5000000;
  q31 b = 2000000;
  q31 result = math.sub<q31>(a, b);
  MU_ASSERT_EQUALS(result, 3000000);
}

static void testMulQ31() {
  q31 a = 1073741824; // 0.5 in Q31
  q31 b = 1073741824; // 0.5 in Q31
  q31 result = math.mul<q31>(a, b);
  // 0.5 * 0.5 = 0.25 = 536870912 in Q31
  MU_ASSERT_EQUALS(result, 536870912);
}

// ============ Float Arithmetic Tests ============

static void testAddFloat() {
  f32 a = 1.5f;
  f32 b = 2.5f;
  f32 result = math.add<f32>(a, b);
  f32 expected = 4.0f;
  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

static void testSubFloat() {
  f32 a = 5.5f;
  f32 b = 2.0f;
  f32 result = math.sub<f32>(a, b);
  f32 expected = 3.5f;
  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

static void testMulFloat() {
  f32 a = 2.0f;
  f32 b = 3.0f;
  f32 result = math.mul<f32>(a, b);
  f32 expected = 6.0f;
  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

// ============ Trigonometric Function Tests ============

static void testSincosFloat() {
  f32 angle = 0.0f; // 0 radians
  f32 modulus = 1.0f;
  auto result = math.sincos<f32>(angle, modulus);

  // sin(0) = 0, cos(0) = 1
  MU_ASSERT(result.v1 < 0.001f && result.v1 > -0.001f); // sin close to 0
  MU_ASSERT((result.v2 - 1.0f) < 0.001f &&
            (result.v2 - 1.0f) > -0.001f); // cos close to 1
}

static void testSincosFloatPiHalf() {
  f32 angle = kPI_2; // π/2 radians
  f32 modulus = 1.0f;
  auto result = math.sincos<f32>(angle, modulus);

  // sin(π/2) = 1, cos(π/2) = 0
  MU_ASSERT((result.v1 - 1.0f) < 0.01f &&
            (result.v1 - 1.0f) > -0.01f);             // sin close to 1
  MU_ASSERT(result.v2 < 0.01f && result.v2 > -0.01f); // cos close to 0
}

static void testSincosWithModulus() {
  f32 angle = 0.0f;
  f32 modulus = 2.0f;
  auto result = math.sincos<f32>(angle, modulus);

  // sin(0) * 2 = 0, cos(0) * 2 = 2
  MU_ASSERT(result.v1 < 0.001f && result.v1 > -0.001f);
  MU_ASSERT((result.v2 - 2.0f) < 0.001f && (result.v2 - 2.0f) > -0.001f);
}

static void testPhaseModulusFloat() {
  f32 x = 1.0f;
  f32 y = 0.0f;
  auto result = math.phaseModulus<f32>(x, y);

  // atan2(0, 1) = 0, sqrt(1^2 + 0^2) = 1
  MU_ASSERT(result.v1 < 0.001f && result.v1 > -0.001f); // phase close to 0
  MU_ASSERT((result.v2 - 1.0f) < 0.001f &&
            (result.v2 - 1.0f) > -0.001f); // modulus close to 1
}

static void testPhaseModulusFloatDiagonal() {
  f32 x = 1.0f;
  f32 y = 1.0f;
  auto result = math.phaseModulus<f32>(x, y);

  // atan2(1, 1) = π/4, sqrt(1^2 + 1^2) = √2
  f32 expected_phase = kPI_4;      // π/4 (if defined) or approximately 0.785398
  f32 expected_modulus = 1.41421f; // √2

  MU_ASSERT((result.v1 - expected_phase) < 0.01f &&
            (result.v1 - expected_phase) > -0.01f);
  MU_ASSERT((result.v2 - expected_modulus) < 0.01f &&
            (result.v2 - expected_modulus) > -0.01f);
}

// ============ Hyperbolic Function Tests ============

static void testSincoshFloat() {
  f32 angle = 0.0f;
  auto result = math.sincosh<f32>(angle);

  // sinh(0) = 0, cosh(0) = 1
  MU_ASSERT(result.v1 < 0.001f && result.v1 > -0.001f); // sinh close to 0
  MU_ASSERT((result.v2 - 1.0f) < 0.001f &&
            (result.v2 - 1.0f) > -0.001f); // cosh close to 1
}

static void testSincoshFloatPositive() {
  f32 angle = 1.0f;
  auto result = math.sincosh<f32>(angle);

  // sinh(1) ≈ 1.1752, cosh(1) ≈ 1.5431
  f32 expected_sinh = 1.1752f;
  f32 expected_cosh = 1.5431f;

  MU_ASSERT((result.v1 - expected_sinh) < 0.01f &&
            (result.v1 - expected_sinh) > -0.01f);
  MU_ASSERT((result.v2 - expected_cosh) < 0.01f &&
            (result.v2 - expected_cosh) > -0.01f);
}

// ============ Log and Square Root Tests ============

static void testSqrtFloat() {
  f32 value = 4.0f;
  f32 result = math.sqrt<f32>(value);
  f32 expected = 2.0f;

  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

static void testSqrtFloatOne() {
  f32 value = 1.0f;
  f32 result = math.sqrt<f32>(value);

  MU_ASSERT((result - 1.0f) < 0.001f && (result - 1.0f) > -0.001f);
}

static void testSqrtFloatSmall() {
  f32 value = 0.25f;
  f32 result = math.sqrt<f32>(value);
  f32 expected = 0.5f;

  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

static void testLogFloat() {
  f32 value = kE; // e ≈ 2.71828
  f32 result = math.log<f32>(value);
  f32 expected = 1.0f;

  MU_ASSERT((result - expected) < 0.01f && (result - expected) > -0.01f);
}

static void testLogFloatOne() {
  f32 value = 1.0f;
  f32 result = math.log<f32>(value);

  MU_ASSERT(result < 0.001f && result > -0.001f); // ln(1) = 0
}

// ============ Sign Tests ============

static void testSignPositive() {
  f32 value = 5.0f;
  f32 result = math.sign<f32>(value);
  MU_ASSERT_EQUALS((i32)result, 1);
}

static void testSignNegative() {
  f32 value = -5.0f;
  f32 result = math.sign<f32>(value);
  MU_ASSERT_EQUALS((i32)result, -1);
}

static void testSignZero() {
  f32 value = 0.0f;
  f32 result = math.sign<f32>(value);
  MU_ASSERT_EQUALS((i32)result, 0);
}

// ============ Normalization Tests ============

static void testCircleNormalizeZero() {
  f32 angle = 0.0f;
  f32 result = math.circleNormalize<f32>(angle);
  MU_ASSERT(result < 0.001f && result > -0.001f);
}

static void testCircleNormalizePi() {
  f32 angle = kPI;
  f32 result = math.circleNormalize<f32>(angle);
  MU_ASSERT((result - kPI) < 0.001f || (result + kPI) < 0.001f); // π or -π
}

static void testCircleNormalize2Pi() {
  f32 angle = k2PI;
  f32 result = math.circleNormalize<f32>(angle);
  MU_ASSERT(result < 0.01f && result > -0.01f); // Should wrap to ~0
}

// ============ Log2 Tests ============

static void testLog2Power() {
  u32 value = 16; // 2^4
  u32 result = math.log2<u32>(value);
  MU_ASSERT_EQUALS(result, 4u);
}

static void testLog2One() {
  u32 value = 1;
  u32 result = math.log2<u32>(value);
  MU_ASSERT_EQUALS(result, 0u);
}

static void testLog2Zero() {
  u32 value = 0;
  u32 result = math.log2<u32>(value);
  MU_ASSERT_EQUALS(result, 0u);
}

// ============ Mod Tests ============

static void testModSimple() {
  f32 x = 10.0f;
  f32 y = 3.0f;
  f32 result = math.mod<f32>(x, y);
  f32 expected = 1.0f;

  MU_ASSERT((result - expected) < 0.001f && (result - expected) > -0.001f);
}

static void testModNegative() {
  f32 x = -10.0f;
  f32 y = 3.0f;
  f32 result = math.mod<f32>(x, y);
  // fmod(-10, 3) = -1, adjusted result should be 2
  MU_ASSERT((result - 2.0f) < 0.001f && (result - 2.0f) > -0.001f);
}

// ============ Test Runner ============

void runMathTests() {
  LOG_I("=== Math Basic Arithmetic Tests ===");
  MU_RUN(testAddQ15);
  MU_RUN(testSubQ15);
  MU_RUN(testMulQ15);
  MU_RUN(testAddQ31);
  MU_RUN(testSubQ31);
  MU_RUN(testMulQ31);

  LOG_I("=== Math Float Arithmetic Tests ===");
  MU_RUN(testAddFloat);
  MU_RUN(testSubFloat);
  MU_RUN(testMulFloat);

  LOG_I("=== Math Trigonometric Tests ===");
  MU_RUN(testSincosFloat);
  MU_RUN(testSincosFloatPiHalf);
  MU_RUN(testSincosWithModulus);
  MU_RUN(testPhaseModulusFloat);
  MU_RUN(testPhaseModulusFloatDiagonal);

  LOG_I("=== Math Hyperbolic Tests ===");
  MU_RUN(testSincoshFloat);
  MU_RUN(testSincoshFloatPositive);

  LOG_I("=== Math Log & Sqrt Tests ===");
  MU_RUN(testSqrtFloat);
  MU_RUN(testSqrtFloatOne);
  MU_RUN(testSqrtFloatSmall);
  MU_RUN(testLogFloat);
  MU_RUN(testLogFloatOne);

  LOG_I("=== Math Sign Tests ===");
  MU_RUN(testSignPositive);
  MU_RUN(testSignNegative);
  MU_RUN(testSignZero);

  LOG_I("=== Math Normalization Tests ===");
  MU_RUN(testCircleNormalizeZero);
  MU_RUN(testCircleNormalizePi);
  MU_RUN(testCircleNormalize2Pi);

  LOG_I("=== Math Log2 Tests ===");
  MU_RUN(testLog2Power);
  MU_RUN(testLog2One);
  MU_RUN(testLog2Zero);

  LOG_I("=== Math Mod Tests ===");
  MU_RUN(testModSimple);
  MU_RUN(testModNegative);

  LOG_I("=== Math Tests Complete ===");
}

} // namespace wibot::test
