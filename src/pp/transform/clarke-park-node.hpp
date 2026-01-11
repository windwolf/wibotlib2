#pragma once

#include "../pipeline.hpp"
#include "dsp/transform/clarke-park.hpp"

namespace wibot {

/**
 * @brief Clarke变换节点 (Clarke Transform Node)
 * 
 * 将三相静止坐标系转换为αβ静止坐标系
 * 用于FOC控制流程中：三相电流采样 → Clarke → Park → PID
 * 
 * 输入:
 *   - ia: A相电流
 *   - ib: B相电流
 * 
 * 输出:
 *   - alpha: α轴分量
 *   - beta: β轴分量
 */
class ClarkeNode : public INode {
   public:
    struct Inputs {
        In<f32> ia;  // A相电流
        In<f32> ib;  // B相电流
    } inputs;

    struct Outputs {
        Out<f32> alpha;  // α轴输出
        Out<f32> beta;   // β轴输出
    } outputs;

    ClarkeNode() = default;

    bool ready() override {
        return inputs.ia.bound() && inputs.ib.bound() && outputs.alpha.bound() &&
               outputs.beta.bound();
    }

    void process() override {
        Clarke::transform(inputs.ia.get(), inputs.ib.get(), outputs.alpha.ref(),
                          outputs.beta.ref());
    }

    void reset() override {
        // 无状态变换，无需重置
    }
};

/**
 * @brief Park变换节点 (Park Transform Node)
 * 
 * 将αβ静止坐标系转换为dq旋转坐标系
 * 用于FOC控制流程中：Clarke → Park → PID
 * 
 * 输入:
 *   - alpha: α轴分量
 *   - beta: β轴分量
 *   - theta: 电角度 (弧度)
 * 
 * 输出:
 *   - d: d轴分量（磁场方向）
 *   - q: q轴分量（转矩方向）
 */
class ParkNode : public INode {
   public:
    struct Inputs {
        In<f32> alpha;  // α轴输入
        In<f32> beta;   // β轴输入
        In<f32> theta;  // 电角度 (弧度)
    } inputs;

    struct Outputs {
        Out<f32> d;  // d轴输出
        Out<f32> q;  // q轴输出
    } outputs;

    ParkNode() = default;

    bool ready() override {
        return inputs.alpha.bound() && inputs.beta.bound() && inputs.theta.bound() &&
               outputs.d.bound() && outputs.q.bound();
    }

    void process() override {
        Park::transform(inputs.alpha.get(), inputs.beta.get(), inputs.theta.get(), outputs.d.ref(),
                        outputs.q.ref());
    }

    void reset() override {
        // 无状态变换，无需重置
    }
};

/**
 * @brief 反Park变换节点 (Inverse Park Transform Node)
 * 
 * 将dq旋转坐标系转换为αβ静止坐标系
 * 用于FOC控制流程中：PID控制器 → InvPark → SVPWM
 * 
 * 输入:
 *   - d: d轴分量 (磁场分量)
 *   - q: q轴分量 (转矩分量)
 *   - theta: 电角度 (弧度)
 * 
 * 输出:
 *   - alpha: α轴分量
 *   - beta: β轴分量
 */
class InvParkNode : public INode {
   public:
    struct Inputs {
        In<f32> d;      // d轴输入
        In<f32> q;      // q轴输入
        In<f32> theta;  // 电角度 (弧度)
    } inputs;

    struct Outputs {
        Out<f32> alpha;  // α轴输出
        Out<f32> beta;   // β轴输出
    } outputs;

    InvParkNode() = default;

    bool ready() override {
        return inputs.d.bound() && inputs.q.bound() && inputs.theta.bound() &&
               outputs.alpha.bound() && outputs.beta.bound();
    }

    void process() override {
        f32 alpha, beta;
        InvPark::transform(inputs.d.get(), inputs.q.get(), inputs.theta.get(), alpha, beta);

        outputs.alpha.ref() = alpha;
        outputs.beta.ref()  = beta;
    }

    void reset() override {
        // 无状态变换，无需重置
    }
};

}  // namespace wibot
