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

### Physical-Model Engine Roadmap

| Engine | State | Intended character |
|---|---|---|
| Karplus-Strong string | Implemented. | Plucked-string excitation and unstable resonant feedback. |
| Digital waveguide string | Planned. | A more explicit travelling-wave string model for guitar-like feedback. |
| Bowed waveguide string | Planned. | Nonlinear bow-friction excitation for sustained bowed-string and cello-like behaviour. |
| Stiff / dispersive string | Planned. | Inharmonic partials and brighter, metallic, or prepared-string behaviour. |
| Modal resonator or multi-resonator bank | Planned. | Dense sympathetic resonance, body-like modes, and drone textures. |
| Custom physical models | Planned. | Future source models selected for Ya-Yu: Reborn and Ya-Yu: Final. |

External input is not itself a physical model. It remains a separate source that can be heard directly, used to excite a selected model, or mixed in parallel with that model.

### Fixed Signal Structure

```text
Source / excitation
    -> Feedback FX slot 1
    -> Feedback FX slot 2
    -> Reverb (when route = Feedback)
    -> feedback delay, gain, and in-loop protection
    -> return to the source
    -> Post FX slot 1
    -> Post FX slot 2
    -> Reverb (when route = Post)
    -> output gain
    -> master limiter
```

An effect participates in feedback by being selected in a feedback slot. An effect outside the loop belongs in a post slot. ToneDrive, Delay, and Flanger remain FX-slot types. Reverb is a single shared processor, outside the slots, with an explicit route selection.

Each chain has two serial slots. Each slot stores its own effect type, dry/wet mix, and ToneDrive parameters.

| Effect type | Status |
|---|---|
| `Bypass` | Implemented. |
| `ToneDrive` | Implemented: three-band EQ, clean, overdrive, distortion, fuzz, and bit crush. |
| `Delay` | Reserved interface; currently safe bypass. |
| `Flanger` | Reserved interface; currently safe bypass. |

The single Reverb has `enabled`, dry/wet mix, feedback, low-pass cutoff, and `route` parameters. `route = Feedback` places it before the body write, so it participates in re-excitation. `route = Post` places it after both Post FX slots, so it affects only final output. Only one route can be active at a time.

### Feedback Protection

The feedback loop uses delay, return gain, and in-loop soft saturation before it re-excites the source. This is separate from the stereo master limiter, which is applied only at final output to protect DAC and monitors.

### Reverb Direction

`ReverbSc` has eight internal modulated delay lines. The project allocates one shared instance, using about `396 KB` of SDRAM. Its route selects feedback-loop or post-output operation; it is not duplicated.

Very long release time is not a primary goal. For Ya-Yu, diffusion, damping, frequency-dependent decay, and a controlled transition into feedback or freeze are more important than a maximum tail measured in seconds.

#### Rings Reverb Reference

The local `ringsX_MIDI` project is a useful compact reference. Its Audrey A/B/C modes reuse the original Mutable Rings reverb: a Dattorro/Griesinger topology with four input all-pass diffusers and a dual cross-feedback tank. It uses one `uint16_t[32768]` delay-memory buffer, about `64 KB` total. The delay states are stored at 16-bit resolution, while the mixing and filter calculations remain floating-point.

This shows that a warm and dense reverb does not require a very large float buffer. The current Audrey mapping has an estimated low-frequency RT60 of roughly 20 seconds at its maximum setting; high frequencies decay sooner through damping. This is already sufficient for ambient and drone use. A local compact Dattorro-style Ya-Yu reverb is therefore a valid future alternative to the shared `ReverbSc` instance. This is a reference and design option, not an implemented Ya-Yu feature.

#### Freeze Direction

Freeze is planned as two distinct behaviours:

1. **Sustained freeze**: latch the current reverb state, fade new input toward zero, and hold the network at a safe near-unity feedback target. It is intended for a continuously sustained texture, with the master limiter and in-loop protection remaining active.
2. **Decay-integrated freeze**: capture the current reverb energy, then smoothly move between normal decay and the held state. Releasing freeze returns to the normal decay target gradually, so the captured sound dissolves into the active reverb rather than stopping or changing abruptly.

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

### 物理建模引擎路线

| 引擎 | 状态 | 预期声音特征 |
|---|---|---|
| Karplus-Strong 弦 | 已实现。 | 拨弦激励与不稳定的共振回授。 |
| 数字波导弦 | 已规划。 | 更明确的行波弦模型，用于类似吉他回授的行为。 |
| 拉弓数字波导弦 | 已规划。 | 通过非线性弓毛摩擦激励，实现持续的弦乐或大提琴类行为。 |
| 刚性 / 色散弦 | 已规划。 | 非谐泛音，更明亮的金属感或预制弦行为。 |
| 模态共振器或多共振器组 | 已规划。 | 浓密的共鸣弦、琴体类模态与 Drone 纹理。 |
| 自定义物理模型 | 已规划。 | 为 Ya-Yu: Reborn 与 Ya-Yu: Final 选择的后续发生源。 |

外部输入本身不是物理建模。它是独立的发生源：可以直接听到、用来激励选定模型，或与模型输出并联混合。

### 固定信号结构

```text
发生源 / 激励
    -> Feedback FX slot 1
    -> Feedback FX slot 2
    -> Reverb（route = Feedback 时）
    -> Feedback delay、增益与回路内保护
    -> 回送至发生源
    -> Post FX slot 1
    -> Post FX slot 2
    -> Reverb（route = Post 时）
    -> 输出增益
    -> Master limiter
```

效果被放入 Feedback 插槽时参与回授；被放入 Post 插槽时位于回路之外。ToneDrive、Delay 和 Flanger 仍属于 FX 插槽类型。Reverb 是独立于插槽之外的一套共享处理器，通过明确的路由参数选择位置。

每条链有两个串联插槽。每个插槽保存自身的效果类型、干湿比与 ToneDrive 参数。

| 效果类型 | 状态 |
|---|---|
| `Bypass` | 已实现。 |
| `ToneDrive` | 已实现：三段 EQ、Clean、Overdrive、Distortion、Fuzz、Bit Crush。 |
| `Delay` | 已预留接口；当前安全直通。 |
| `Flanger` | 已预留接口；当前安全直通。 |

这套 Reverb 具有 `enabled`、干湿比、feedback、低通截止频率和 `route` 参数。`route = Feedback` 时它位于 body 写入之前，参与重新激励；`route = Post` 时它位于两个 Post FX 插槽之后，只影响最终输出。两种路由一次只能选择一种。

### 回授保护

反馈回路在重新激励发生源之前使用延迟、回送增益和回路内 Soft Saturation。这与最终输出的立体声 Master Limiter 独立；后者只负责保护 DAC 与监听系统。

### 混响方向

`ReverbSc` 内部有八条带调制的延迟线。项目只分配一套共享实例，约使用 `396 KB` SDRAM；通过路由选择它在 Feedback loop 内还是在最终输出后工作，不再复制实例。

不应将极长的 Release 作为首要目标。对于 Ya-Yu，扩散、阻尼、随频率变化的衰减，以及进入 Feedback 或 Freeze 时的平滑过渡，比以秒数衡量的最长尾音更重要。

#### Rings 混响参考

本地 `ringsX_MIDI` 项目是一个很有价值的紧凑参考。它的 Audrey A/B/C 模式复用了原版 Mutable Rings 混响：Dattorro/Griesinger 拓扑，包含四级输入 all-pass 扩散器和双交叉反馈 tank。它只使用一块 `uint16_t[32768]` 延迟缓冲，总计约 `64 KB`。延迟线状态以 16-bit 保存，但混合与滤波计算仍使用浮点。

这说明温暖、浓密的混响不一定需要很大的 float 缓冲。当前 Audrey 映射在最大值时，低频 RT60 估算约为 20 秒；高频会因阻尼更早衰减。这对 Ambient 与 Drone 已经足够。未来可在 Ya-Yu 内部实现紧凑的 Dattorro 风格混响，用来替换这套共享的 `ReverbSc`。这只是参考与设计选项，不是当前已实现功能。

#### Freeze 方向

Freeze 计划为两种不同的行为：

1. **持续冻结**：锁存当前混响状态，将新的输入平滑淡出，并把网络保持在安全、接近 1 的反馈目标。它用于持续维持音色纹理，Master Limiter 与回路内保护仍保持工作。
2. **融入自然衰减的冻结**：捕获当前混响能量，再在正常衰减与保持状态之间平滑移动。释放 Freeze 后，反馈目标逐渐回到正常衰减，使冻结的声音自然融入当前混响，而不是突然停止或突变。

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
