# Ya-Yu

## English

Ya-Yu is an experimental physical-model feedback instrument and effect project, named after the mythological Yà-Yǔ (猰貐). It is inspired by [Audrey II from Synthux Academy](https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii), especially its Karplus-Strong sound design and nested-feedback ideas.

The main goal is controllable guitar-style feedback: external sound or a physical model is shaped, then a controlled portion returns to re-excite the source. The system should support clean sustaining resonance, dense drones, and unstable nonlinear feedback without losing musical intent.

### Three Forms

| Form | Direction |
|---|---|
| **Ya-Yu: Prime** | The original complete form before rebirth. A compact SpotykachX engine with clean, airy, resonant sound; distortion is light or can be disabled. |
| **Ya-Yu: Reborn** | The reborn beast form. The full dedicated instrument with stronger nonlinear feedback, distortion, fuzz, dense ambience, and more forceful drones. |
| **Ya-Yu: Final** | The final state after the encounter with Hou Yi. A later experimental form for fragmented, destructive, unstable, and other unconventional sound processes. |


Implementation details and development route are in [`TECHNICAL.md`](TECHNICAL.md).

This is version **0.1** and is released under the [MIT License](LICENSE).

### Firmware Builds

| Command | Output | Target |
|---|---|---|
| `make seed` | `build/seed/yayu_seed.bin` | Original Daisy bootloader (`BOOT_QSPI`) |
| `make spotykach` | `build/spotykach/yayu_spotykach.bin` | Spotykach SRAM application bootloader profile (`BOOT_SRAM`) |

Flash commands are `make flash-seed` and `make flash-spotykach`. They only write an application image; they do not build or overwrite a bootloader. No shared library source is modified by this project.

Clone with dependencies using `git clone --recurse-submodules <repository-url>`, then run `make seed` or `make spotykach`. `libDaisy` and `DaisySP` are linked as Git submodules; their source is not modified.

## 中文

Ya-Yu 是一个实验性的物理建模回授乐器与效果器项目，名称来自中国神话人物猰貐（Yà-Yǔ）。项目受到 [Synthux Academy 的 Audrey II](https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii) 的物理建模与嵌套回授思路启发，尤其参考其 Karplus-Strong 声音设计。

主要目标是可控制的吉他回授：外部声音或物理建模发声源经过音色塑形后，其中受控的一部分重新返回并激励发声源。系统应能实现干净的持续共振、浓密 Drone 与不稳定的非线性回授，同时保持音乐性。

### 三个形态

| 形态 | 方向 |
|---|---|
| **Ya-Yu: Prime** | 复生前完整的原初形态。SpotykachX 中的紧凑引擎，声音偏干净、空灵和共振；失真轻微，也可以完全关闭。 |
| **Ya-Yu: Reborn** | 复生后化为异兽。完整的独立乐器，加入更强的非线性回授、失真、Fuzz、浓密空间感和更有冲击力的 Drone。 |
| **Ya-Yu: Final** | 与后羿相遇之后的最终状态。后续的实验形态，用于碎片化、破坏性、不稳定与其他非常规声音过程。 |


实现细节与开发路线见 [`TECHNICAL.md`](TECHNICAL.md)。

当前为 **0.1** 版本，采用 [MIT License](LICENSE)。

### 固件构建

| 命令 | 输出 | 目标 |
|---|---|---|
| `make seed` | `build/seed/yayu_seed.bin` | 原版 Daisy bootloader（`BOOT_QSPI`） |
| `make spotykach` | `build/spotykach/yayu_spotykach.bin` | Spotykach SRAM 应用 bootloader 配置（`BOOT_SRAM`） |

烧录命令为 `make flash-seed` 与 `make flash-spotykach`。它们只写入应用固件，不会构建或覆盖 bootloader。本项目不会修改任何共享库源码。

请使用 `git clone --recurse-submodules <repository-url>` 克隆包含依赖的项目，然后运行 `make seed` 或 `make spotykach`。`libDaisy` 和 `DaisySP` 通过 Git submodule 链接，不会修改它们的源码。
