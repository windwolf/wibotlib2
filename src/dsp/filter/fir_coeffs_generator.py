#!/usr/bin/env python3
"""
FIR 滤波器系数生成器

使用 Parks–McClellan (Remez) 算法设计等波纹 FIR 滤波器，
生成可直接用于 C++ 代码的系数数组。

依赖: pip install scipy numpy

使用示例:
    python fir_coeffs_generator.py
"""

import numpy as np
from scipy import signal


def design_lowpass_pm(tap_count, cutoff_freq, sample_freq, transition_width=0.1):
    """
    使用 Parks–McClellan 算法设计低通滤波器
    
    Args:
        tap_count: 抽头数量（建议奇数）
        cutoff_freq: 截止频率 (Hz)
        sample_freq: 采样频率 (Hz)
        transition_width: 过渡带宽度（相对于采样频率的比例，默认 0.1）
    
    Returns:
        numpy array: 滤波器系数
    """
    # 归一化频率
    nyquist = sample_freq / 2.0
    pass_edge = cutoff_freq / nyquist
    stop_edge = pass_edge + transition_width
    
    # 确保频率在有效范围内
    if stop_edge >= 1.0:
        stop_edge = 0.99
        pass_edge = stop_edge - transition_width
    
    # Parks–McClellan 算法
    bands = [0, pass_edge, stop_edge, 1.0]
    desired = [1, 0]  # 通带增益1，阻带增益0
    
    coeffs = signal.remez(tap_count, bands, desired, fs=2.0)
    
    return coeffs


def design_lowpass_window(tap_count, cutoff_freq, sample_freq, window='hamming'):
    """
    使用窗函数法设计低通滤波器
    
    Args:
        tap_count: 抽头数量
        cutoff_freq: 截止频率 (Hz)
        sample_freq: 采样频率 (Hz)
        window: 窗函数类型 ('hamming', 'hann', 'blackman', 'rectangular')
    
    Returns:
        numpy array: 滤波器系数
    """
    nyquist = sample_freq / 2.0
    normalized_cutoff = cutoff_freq / nyquist
    
    if window == 'rectangular':
        window_func = 'boxcar'
    elif window == 'hann':
        window_func = 'hann'
    elif window == 'blackman':
        window_func = 'blackman'
    else:
        window_func = 'hamming'
    
    coeffs = signal.firwin(tap_count, normalized_cutoff, window=window_func)
    
    return coeffs


def generate_cpp_code(coeffs, var_name="fir_coeffs"):
    """
    生成 C++ 代码
    
    Args:
        coeffs: 滤波器系数数组
        var_name: 变量名
    
    Returns:
        str: C++ 代码字符串
    """
    cpp_code = f"// FIR 滤波器系数 (N={len(coeffs)})\n"
    cpp_code += f"constexpr f32 {var_name}[{len(coeffs)}] = {{\n"
    
    # 每行8个系数
    for i in range(0, len(coeffs), 8):
        line_coeffs = coeffs[i:i+8]
        cpp_code += "    " + ", ".join(f"{c:+.10f}f" for c in line_coeffs)
        if i + 8 < len(coeffs):
            cpp_code += ","
        cpp_code += "\n"
    
    cpp_code += "};\n"
    
    return cpp_code


def analyze_filter(coeffs, sample_freq):
    """
    分析滤波器特性
    """
    # 频率响应
    w, h = signal.freqz(coeffs, worN=8000)
    freq = w * sample_freq / (2 * np.pi)
    
    # 计算通带衰减和阻带衰减
    gain_db = 20 * np.log10(np.abs(h))
    
    print(f"\n滤波器分析:")
    print(f"  抽头数量: {len(coeffs)}")
    print(f"  系数和 (DC增益): {np.sum(coeffs):.6f}")
    print(f"  最大增益: {np.max(gain_db):.2f} dB")
    print(f"  通带波纹: ±{np.max(np.abs(gain_db[:len(gain_db)//4])):.2f} dB")


def main():
    """主函数 - 示例用法"""
    
    # 设计参数
    SAMPLE_FREQ = 20000.0  # 20 kHz 采样率
    CUTOFF_FREQ = 1000.0   # 1 kHz 截止频率
    TAP_COUNT = 21         # 21 抽头
    
    print("=" * 60)
    print("FIR 滤波器系数生成器")
    print("=" * 60)
    
    # 方法 1: Parks–McClellan (推荐)
    print("\n[1] Parks–McClellan 算法 (等波纹)")
    coeffs_pm = design_lowpass_pm(TAP_COUNT, CUTOFF_FREQ, SAMPLE_FREQ, transition_width=0.15)
    analyze_filter(coeffs_pm, SAMPLE_FREQ)
    print(generate_cpp_code(coeffs_pm, "pm_lpf_1khz"))
    
    # 方法 2: 窗函数法 (Hamming)
    print("\n[2] 窗函数法 (Hamming)")
    coeffs_hamming = design_lowpass_window(TAP_COUNT, CUTOFF_FREQ, SAMPLE_FREQ, 'hamming')
    analyze_filter(coeffs_hamming, SAMPLE_FREQ)
    print(generate_cpp_code(coeffs_hamming, "hamming_lpf_1khz"))
    
    # 方法 3: 窗函数法 (Blackman) - 更好的阻带衰减
    print("\n[3] 窗函数法 (Blackman)")
    coeffs_blackman = design_lowpass_window(TAP_COUNT, CUTOFF_FREQ, SAMPLE_FREQ, 'blackman')
    analyze_filter(coeffs_blackman, SAMPLE_FREQ)
    print(generate_cpp_code(coeffs_blackman, "blackman_lpf_1khz"))
    
    print("\n" + "=" * 60)
    print("提示: 将生成的代码复制到 C++ 项目中使用")
    print("=" * 60)


if __name__ == "__main__":
    main()
