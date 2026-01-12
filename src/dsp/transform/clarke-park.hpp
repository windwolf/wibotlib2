#pragma once

#include "base/type.hpp"
#include "math/index.hpp"

namespace wibot {

/**
 * @brief Clarke恒功率变换 (Clarke Constant Power Transform)
 * 
 * 将三相静止坐标系（ABC）转换到两相静止坐标系（αβ）
 * 保持功率守恒：$P_{abc} = P_{\\alpha\\beta}$
 * 
 * 变换公式（恒功率型）：
 *   i_alpha = i_a
 *   i_beta = (i_a + 2*i_b) / √3
 * 
 * 特点：
 * - 功率守恒，符合能量转换原理
 * - 广泛用于标准 FOC 电机控制
 * - PI 控制参数与工业标准一致
 * 
 * 利用基尔霍夫电流定律：i_a + i_b + i_c = 0，只需要两相电流
 * 
 * @note 这是无状态变换，提供静态方法直接调用
 */
class ClarkeConstPower {
   public:
    /**
     * @brief 执行Clarke恒功率变换（两相电流输入，向量返回）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @return αβ坐标系输出向量 [alpha, beta]
     */
    static Vector2f transform(f32 ia, f32 ib) {
        Vector2f result;
        result.v1 = ia;                           // alpha = ia
        result.v2 = (ia + 2.0f * ib) * k1_SQRT3;  // beta = (ia + 2*ib) / √3
        return result;
    }

    /**
     * @brief 执行Clarke恒功率变换（两相电流输入，引用输出）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @param alpha [out] α轴输出
     * @param beta [out] β轴输出
     */
    static void transform(f32 ia, f32 ib, f32& alpha, f32& beta) {
        alpha = ia;
        beta  = (ia + 2.0f * ib) * k1_SQRT3;
    }

    /**
     * @brief 执行Clarke恒功率变换（三相电流输入，向量返回）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @param ic C相电流
     * @return αβ坐标系输出向量 [alpha, beta]
     */
    static Vector2f transform(f32 ia, f32 ib, f32 ic) {
        Vector2f result;
        result.v1 = ia;
        result.v2 = (ia + 2.0f * ib) * k1_SQRT3;
        return result;
    }
};

/**
 * @brief Clarke恒幅值变换 (Clarke Constant Magnitude Transform)
 * 
 * 将三相静止坐标系（ABC）转换到两相静止坐标系（αβ）
 * 保持幅值守恒：$|\\mathbf{I}_{abc}| = |\\mathbf{I}_{\\alpha\\beta}|$
 * 
 * 变换公式（恒幅值型）：
 *   i_alpha = √(2/3) * i_a
 *   i_beta = √(2/9) * (i_a + 2*i_b)
 * 
 * 特点：
 * - 幅值守恒，便于信号分析和诊断
 * - 用于信号处理和故障检测场景
 * - 适合非线性控制算法
 * 
 * 利用基尔霍夫电流定律：i_a + i_b + i_c = 0，只需要两相电流
 * 
 * @note 这是无状态变换，提供静态方法直接调用
 */
class ClarkeConstMag {
   public:
    /**
     * @brief 执行Clarke恒幅值变换（两相电流输入，向量返回）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @return αβ坐标系输出向量 [alpha, beta]
     */
    static Vector2f transform(f32 ia, f32 ib) {
        Vector2f result;
        result.v1 = ia * kSQRT2_3;                // alpha = √(2/3) * ia
        result.v2 = (ia + 2.0f * ib) * kSQRT2_9;  // beta = √(2/9) * (ia + 2*ib)
        return result;
    }

    /**
     * @brief 执行Clarke恒幅值变换（两相电流输入，引用输出）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @param alpha [out] α轴输出
     * @param beta [out] β轴输出
     */
    static void transform(f32 ia, f32 ib, f32& alpha, f32& beta) {
        alpha = ia * kSQRT2_3;
        beta  = (ia + 2.0f * ib) * kSQRT2_9;
    }

    /**
     * @brief 执行Clarke恒幅值变换（三相电流输入，向量返回）
     * 
     * @param ia A相电流
     * @param ib B相电流
     * @param ic C相电流
     * @return αβ坐标系输出向量 [alpha, beta]
     */
    static Vector2f transform(f32 ia, f32 ib, f32 ic) {
        Vector2f result;
        result.v1 = ia * kSQRT2_3;
        result.v2 = (ia + 2.0f * ib) * kSQRT2_9;
        return result;
    }
};

/**
 * @brief Park变换 (Park Transform)
 * 
 * 将两相静止坐标系（αβ）转换到两相旋转坐标系（dq）
 * 用于FOC控制中将αβ坐标系电流/电压转换为dq坐标系
 * 
 * 变换公式 (标准形式):
 *   i_d = i_alpha * cos(theta) + i_beta * sin(theta)
 *   i_q = -i_alpha * sin(theta) + i_beta * cos(theta)
 * 
 * 变换矩阵形式:
 *   [d]   [ cos(θ)   sin(θ)] [alpha]
 *   [q] = [-sin(θ)   cos(θ)] [beta ]
 * 
 * @note Park变换与Clarke变换的选择无关（等幅值或等功率）
 *       Park变换仅执行坐标系旋转，不涉及幅值变换
 * @note 这是无状态变换，提供静态方法直接调用
 */
class Park {
   public:
    /**
     * @brief 执行Park变换（向量输入形式）
     * 
     * @param alphaBeta αβ坐标系输入向量 [alpha, beta]
     * @param theta 电角度 (弧度)
     * @return dq坐标系输出向量 [d, q]
     */
    static Vector2f transform(const Vector2f& alphaBeta, f32 theta) {
        Math      math;
        Vector2f  sc       = math.sincos<f32>(theta, 1.0f);  // v1=sin, v2=cos
        const f32 sinTheta = sc.v1;
        const f32 cosTheta = sc.v2;

        Vector2f result;
        result.v1 = alphaBeta.v1 * cosTheta + alphaBeta.v2 * sinTheta;   // d
        result.v2 = -alphaBeta.v1 * sinTheta + alphaBeta.v2 * cosTheta;  // q

        return result;
    }

    /**
     * @brief 执行Park变换（分量形式，最高效）
     * 
     * 推荐在实时控制环路中使用此方法
     * 
     * @param alpha α轴分量
     * @param beta β轴分量
     * @param theta 电角度 (弧度)
     * @param d [out] d轴输出
     * @param q [out] q轴输出
     */
    static void transform(f32 alpha, f32 beta, f32 theta, f32& d, f32& q) {
        Math      math;
        Vector2f  sc       = math.sincos<f32>(theta, 1.0f);  // v1=sin, v2=cos
        const f32 sinTheta = sc.v1;
        const f32 cosTheta = sc.v2;

        d = alpha * cosTheta + beta * sinTheta;
        q = -alpha * sinTheta + beta * cosTheta;
    }

    /**
     * @brief 执行Park变换（使用预先计算的sin/cos值，最高效）
     * 
     * 当需要多次使用相同角度时，可预先计算sin/cos值以提升性能
     * 
     * @param alpha α轴分量
     * @param beta β轴分量
     * @param cosTheta cos(theta)
     * @param sinTheta sin(theta)
     * @param d [out] d轴输出
     * @param q [out] q轴输出
     */
    static void transformWithSinCos(f32 alpha, f32 beta, f32 cosTheta, f32 sinTheta, f32& d,
                                    f32& q) {
        d = alpha * cosTheta + beta * sinTheta;
        q = -alpha * sinTheta + beta * cosTheta;
    }
};

/**
 * @brief 反Park变换 (Inverse Park Transform)
 * 
 * 将dq旋转坐标系的量转换到αβ静止坐标系
 * 用于FOC控制中将电流环输出的dq轴电压转换为αβ轴电压
 * 
 * 变换公式 (Park变换的逆变换):
 *   u_alpha = u_d * cos(theta) - u_q * sin(theta)
 *   u_beta  = u_d * sin(theta) + u_q * cos(theta)
 * 
 * 变换矩阵形式:
 *   [alpha]   [cos(θ)  -sin(θ)] [d]
 *   [beta ] = [sin(θ)   cos(θ)] [q]
 * 
 * @note 反Park变换与Clarke变换的选择无关
 *       反Park变换仅执行坐标系旋转，不涉及幅值变换
 * @note 这是无状态变换，提供静态方法直接调用
 */
class InvPark {
   public:
    /**
     * @brief 执行反Park变换（向量输入形式）
     * 
     * @param dq dq坐标系输入向量 [d, q]
     * @param theta 电角度 (弧度)
     * @return αβ坐标系输出向量 [alpha, beta]
     */
    static Vector2f transform(const Vector2f& dq, f32 theta) {
        Math      math;
        Vector2f  sc       = math.sincos<f32>(theta, 1.0f);  // v1=sin, v2=cos
        const f32 sinTheta = sc.v1;
        const f32 cosTheta = sc.v2;

        Vector2f result;
        result.v1 = dq.v1 * cosTheta - dq.v2 * sinTheta;  // alpha
        result.v2 = dq.v1 * sinTheta + dq.v2 * cosTheta;  // beta

        return result;
    }

    /**
     * @brief 执行反Park变换 (分量形式，最高效)
     * 
     * 推荐在实时控制环路中使用此方法
     * 
     * @param d d轴分量
     * @param q q轴分量
     * @param theta 电角度 (弧度)
     * @param alpha [out] α轴输出
     * @param beta [out] β轴输出
     */
    static void transform(f32 d, f32 q, f32 theta, f32& alpha, f32& beta) {
        Math      math;
        Vector2f  sc       = math.sincos<f32>(theta, 1.0f);  // v1=sin, v2=cos
        const f32 sinTheta = sc.v1;
        const f32 cosTheta = sc.v2;

        alpha = d * cosTheta - q * sinTheta;
        beta  = d * sinTheta + q * cosTheta;
    }

    /**
     * @brief 执行反Park变换 (使用预先计算的sin/cos值，最高效)
     * 
     * 当需要多次使用相同角度时，可预先计算sin/cos值以提升性能
     * @param beta [out] β轴输出
     */
    static void transformWithSinCos(f32 d, f32 q, f32 cosTheta, f32 sinTheta, f32& alpha,
                                    f32& beta) {
        alpha = d * cosTheta - q * sinTheta;
        beta  = d * sinTheta + q * cosTheta;
    }
};

}  // namespace wibot
