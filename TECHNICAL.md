# Ya-Yu Technical Notes

## English

### Current DSP Framework

The current code is a framework, not a complete instrument. It has no panel, CV, MIDI, preset, or hardware-specific control mapping.

Implemented source modes:

- Direct external input.
- Karplus-Strong model only.
- External input exciting Karplus-Strong.
- External input and Karplus-Strong in parallel.

Digital waveguide, bowed-string, stiff-string, modal resonator, and other physical models are planned but are not implemented.

### Fixed Signal Structure

```text
Source / excitation
    -> Feedback FX slot 1
    -> Feedback FX slot 2
    -> feedback delay, gain, and in-loop protection
    -> return to the source
    -> Post FX slot 1
    -> Post FX slot 2
    -> output gain
    -> master limiter
```

An effect participates in feedback by being selected in a feedback slot. An effect outside the loop belongs in a post slot. There is no global before/after routing switch.

Each chain has two serial slots. Each slot stores its own effect type, dry/wet mix, ToneDrive parameters, and Reverb parameters.

| Effect type | Status |
|---|---|
| `Bypass` | Implemented. |
| `ToneDrive` | Implemented: three-band EQ, clean, overdrive, distortion, fuzz, and bit crush. |
| `Reverb` | Implemented: one independent `ReverbSc` instance per chain. |
| `Delay` | Reserved interface; currently safe bypass. |
| `Flanger` | Reserved interface; currently safe bypass. |

Only one Reverb can be active per chain for now. A second Reverb selection in the same chain is automatically bypassed because each chain currently owns one reverb instance.

### Feedback Protection

The feedback loop uses delay, return gain, and in-loop soft saturation before it re-excites the source. This is separate from the stereo master limiter, which is applied only at final output to protect DAC and monitors.

### Reverb Direction

`ReverbSc` has eight internal modulated delay lines. The project currently allocates one instance for the feedback chain and one for the post chain, using about `792 KB` of SDRAM together.

Future warm/drone reverb work should be implemented locally in Ya-Yu, not by changing DaisySP: a dark `ReverbSc` parameter mode, then a dedicated 12/16-line FDN with high-frequency damping, low-frequency control, shallow modulation, and stable feedback protection.

### Firmware Profiles

| Command | Output | Application layout |
|---|---|---|
| `make seed` | `build/seed/yayu_seed.bin` | Original Daisy `BOOT_QSPI` application. |
| `make spotykach` | `build/spotykach/yayu_spotykach.bin` | Spotykach SRAM application layout compatible with the local Spotykach 1.1 project. |

The Spotykach target uses the local application linker layout and references the locally available `bootloader-spotykach-v2.bin`; it does not overwrite that bootloader.

### Development Route

1. Turn **Ya-Yu: Prime** into a compact SpotykachX Deck A engine.
2. Add selected physical models and practical FX implementations.
3. Build **Ya-Yu: Reborn** as a dedicated hardware version with deeper control and larger DSP scope.
4. Explore the more experimental **Ya-Yu: Final** direction separately.

### Library Boundary

Karplus-Strong was copied from [Synthux Academy's original Audrey II project](https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii) with its MIT notice retained. DaisySP and libDaisy are linked as Git submodules. Their source code is not modified by Ya-Yu.

## 中文

### 当前 DSP 框架

当前代码是框架，不是完整乐器。它没有面板、CV、MIDI、预设或任何硬件专用控制映射。

已实现的发生源模式：

- 外部输入直通。
- 仅 Karplus-Strong 模型。
- 外部输入激励 Karplus-Strong。
- 外部输入与 Karplus-Strong 并联。

数字波导、拉弓弦、刚性弦、模态共振器及其他物理模型已经规划，但尚未实现。

### 固定信号结构

```text
发生源 / 激励
    -> Feedback FX slot 1
    -> Feedback FX slot 2
    -> Feedback delay、增益与回路内保护
    -> 回送至发生源
    -> Post FX slot 1
    -> Post FX slot 2
    -> 输出增益
    -> Master limiter
```

效果被放入 Feedback 插槽时参与回授；被放入 Post 插槽时位于回路之外。没有全局的前后路由开关。

每条链有两个串联插槽。每个插槽保存自身的效果类型、干湿比、ToneDrive 参数与 Reverb 参数。

| 效果类型 | 状态 |
|---|---|
| `Bypass` | 已实现。 |
| `ToneDrive` | 已实现：三段 EQ、Clean、Overdrive、Distortion、Fuzz、Bit Crush。 |
| `Reverb` | 已实现：每条链各有一套独立 `ReverbSc`。 |
| `Delay` | 已预留接口；当前安全直通。 |
| `Flanger` | 已预留接口；当前安全直通。 |

当前每条链只能启用一个 Reverb。同一条链的第二个 Reverb 选择会自动直通，因为每条链目前只分配了一套混响实例。

### 回授保护

反馈回路在重新激励发生源之前使用延迟、回送增益和回路内 Soft Saturation。这与最终输出的立体声 Master Limiter 独立；后者只负责保护 DAC 与监听系统。

### 混响方向

`ReverbSc` 内部有八条带调制的延迟线。项目目前给 Feedback chain 和 Post chain 各分配一套实例，两者合计约使用 `792 KB` SDRAM。

后续温暖/Drone 混响应在 Ya-Yu 项目内实现，不修改 DaisySP：先增加暗色 `ReverbSc` 参数模式，再实现专用的 12/16 线 FDN，加入高频阻尼、低频控制、浅调制与稳定的回路保护。

### 固件配置

| 命令 | 输出 | 应用布局 |
|---|---|---|
| `make seed` | `build/seed/yayu_seed.bin` | 原版 Daisy `BOOT_QSPI` 应用。 |
| `make spotykach` | `build/spotykach/yayu_spotykach.bin` | 与本地 Spotykach 1.1 工程兼容的 Spotykach SRAM 应用布局。 |

Spotykach 目标使用本地应用 linker 布局，并引用本地已有的 `bootloader-spotykach-v2.bin`；不会覆盖该 bootloader。

### 开发路线

1. 将 **Ya-Yu: Prime** 做成紧凑的 SpotykachX Deck A 引擎。
2. 加入选定的物理模型和实用 FX 实现。
3. 将 **Ya-Yu: Reborn** 做成具有更深控制和更大 DSP 范围的独立硬件版本。
4. 单独探索更实验性的 **Ya-Yu: Final** 方向。

### 库边界

Karplus-Strong 从 [Synthux Academy 原版 Audrey II 项目](https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii) 复制，并保留其 MIT 声明。DaisySP 和 libDaisy 通过 Git submodule 链接；Ya-Yu 不修改它们的源码。
