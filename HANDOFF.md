# GeoTaskShield 项目交接文档

更新时间：2026-04-27  
项目路径：`D:\VS2026_Projects\GeoTaskShield`  
当前状态：阶段 1、阶段 2 已完成并通过验收。阶段 3 尚未开始。

---

## 1. 项目概述

GeoTaskShield 是一个面向移动群智感知场景的隐私保护任务分配仿真系统。当前版本以 C++20/CMake 控制台程序为主，核心逻辑保持 Qt 无关，为后续 Qt GUI 可视化开发预留接口。

系统当前支持：

- 生成模拟任务与移动用户；
- 对用户位置进行隐私保护；
- 使用不同任务分配算法完成任务匹配；
- 计算完成率、平均真实移动距离、总收益、隐私损失和算法运行时间；
- 在控制台输出算法对比结果；
- 导出 CSV 实验结果。

---

## 2. 当前工程结构

```text
GeoTaskShield/
  CMakeLists.txt
  CMakePresets.json
  GeoTaskShield.md
  HANDOFF.md
  task_plan.md
  findings.md
  progress.md
  phase2_results.csv
  GeoTaskShield/
    CMakeLists.txt
    app/
      console/main.cpp
    model/
    simulation/
    privacy/
    assignment/
    evaluation/
    data/
    tests/test_core.cpp
```

核心目录说明：

| 目录 | 说明 |
|---|---|
| `model` | 基础数据模型，如任务、用户、位置、配置、分配结果 |
| `simulation` | 数据生成与仿真实验编排 |
| `privacy` | 位置隐私保护策略与工厂 |
| `assignment` | 任务分配算法策略与工厂 |
| `evaluation` | 实验指标计算 |
| `data` | CSV 结果导出 |
| `app/console` | 控制台入口 |
| `tests` | 当前核心测试入口 |

---

## 3. 阶段 1 完成情况

阶段 1 目标：完成最小可运行控制台 MVP，跑通“生成数据 -> 网格隐私 -> 最近贪心分配 -> 指标评估 -> 控制台输出”链路。

已完成内容：

- 数据模型：
  - `Location`
  - `Task`
  - `Worker`
  - `Assignment`
  - `ExperimentConfig`
- 仿真数据生成：
  - `DataGenerator`
- 隐私机制：
  - `IPrivacyMechanism`
  - `GridPrivacy`
- 分配算法：
  - `IAssignmentAlgorithm`
  - `NearestGreedyAlgorithm`
- 指标评估：
  - `EvaluationMetrics`
  - `MetricsCalculator`
- 仿真编排：
  - `SimulationEngine`
- 控制台入口：
  - `app/console/main.cpp`
- CMake 结构：
  - `GeoTaskShieldCore` 静态库
  - `GeoTaskShield` 控制台可执行程序
  - `GeoTaskShieldTests` 测试目标

阶段 1 验收结果：

- 可生成 100 个用户和 50 个任务；
- 可启用网格隐私；
- 可执行最近距离贪心分配；
- 可输出完成率、平均移动距离、总收益、隐私损失、运行时间；
- 核心逻辑不依赖 Qt。

---

## 4. 阶段 2 完成情况

阶段 2 目标：完善隐私机制、分配算法、对比实验能力和结果导出能力。

### 4.1 新增隐私机制

| 类 | 说明 |
|---|---|
| `KAnonymityPrivacy` | 将每个用户暴露位置映射为其最近 k 个用户真实位置的质心 |
| `LaplaceNoisePrivacy` | 对用户坐标加入确定性随机种子的 Laplace 噪声 |
| `PrivacyFactory` | 根据字符串创建隐私机制 |

当前支持的隐私策略字符串：

```text
grid
k-anonymity
kanonymity
k
laplace
laplace-noise
```

### 4.2 新增分配算法

| 类 | 说明 |
|---|---|
| `ScoreGreedyAlgorithm` | 使用可靠度、奖励、距离、隐私预算构造评分并贪心分配 |
| `HungarianAlgorithm` | 基于扩展 worker slot 的一对一最小代价匹配 |
| `AssignmentAlgorithmFactory` | 根据字符串创建分配算法 |

当前支持的算法策略字符串：

```text
nearest
nearest-greedy
score
score-greedy
hungarian
```

### 4.3 控制台对比实验

控制台程序现在运行 3 x 3 对比矩阵：

```text
隐私机制：
- Grid Privacy
- K-Anonymity Privacy
- Laplace Noise Privacy

分配算法：
- Nearest Greedy
- Score Greedy
- Hungarian
```

输出指标包括：

- 已完成任务数；
- 完成率；
- 平均真实移动距离；
- 总收益；
- 平均隐私损失；
- 算法运行时间。

### 4.4 CSV 导出

新增：

- `ExperimentSummaryRow`
- `CsvExporter::toCsv`
- `CsvExporter::writeToFile`

控制台运行后会在项目根目录生成：

```text
phase2_results.csv
```

当前 CSV 包含 1 行表头和 9 行实验结果。

---

## 5. 构建与验收方式

推荐在 Visual Studio Developer Command Prompt 或通过 `VsDevCmd.bat` 加载 MSVC 环境后执行。

当前已验证命令：

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure && out\build\x64-debug\GeoTaskShield\GeoTaskShield.exe'
```

验收结果摘要：

```text
1/1 Test #1: GeoTaskShieldCoreTests Passed
100% tests passed, 0 tests failed out of 1
```

控制台会输出 9 组对比实验结果，并生成：

```text
D:\VS2026_Projects\GeoTaskShield\phase2_results.csv
```

注意：

- 普通 PowerShell 环境可能找不到 `cl.exe` 或 `ninja`；
- 如果使用 Qt 安装器自带 CMake/Ninja，需要确保其路径已加入 PATH，或直接用 Visual Studio 的 CMake/Ninja 环境。

---

## 6. 当前设计约束与注意事项

1. 核心逻辑保持 Qt 无关。
   - `model`、`simulation`、`privacy`、`assignment`、`evaluation`、`data` 均为纯 C++。

2. 当前测试是轻量自定义测试。
   - 文件：`GeoTaskShield/tests/test_core.cpp`
   - 未引入 GoogleTest/Catch2。

3. Hungarian 算法当前语义：
   - 支持一对一任务匹配；
   - 通过展开 `Worker::maxTasks` 为多个 worker slot 支持简单容量；
   - 不支持一个任务需要多个 worker 的严格优化建模。

4. Laplace 噪声当前语义：
   - 使用固定随机种子，保证测试和实验可复现；
   - 属于仿真意义上的坐标扰动，不是完整差分隐私证明实现。

5. `GeoTaskShield.cpp` 和 `GeoTaskShield.h` 仍保留在目录中，但当前 CMake 使用 `app/console/main.cpp` 作为入口。

---

## 7. 后续开发计划

### 阶段 3：Qt GUI 可视化

目标：把当前控制台实验能力接入 Qt Widgets 桌面界面。

建议任务：

1. CMake 接入 Qt Widgets。
2. 新增 GUI 目录：

```text
GeoTaskShield/gui/
  MainWindow.h/.cpp
  MapCanvas.h/.cpp
  ParameterPanel.h/.cpp
  ResultPanel.h/.cpp
  LogPanel.h/.cpp
```

3. 设计主窗口布局：
   - 左侧参数面板；
   - 中间二维地图画布；
   - 右侧指标结果面板；
   - 底部日志面板。
4. 参数面板接入：
   - worker 数量；
   - task 数量；
   - grid size；
   - k；
   - epsilon；
   - 隐私机制；
   - 分配算法。
5. `MapCanvas` 绘制：
   - worker 蓝点；
   - task 红点；
   - assignment 虚线；
   - grid/k-anonymity 区域可后续增强。
6. GUI 只调用 `SimulationEngine` 和工厂，不直接写算法逻辑。

阶段 3 验收标准：

- GUI 能启动；
- 用户可选择隐私机制和算法；
- 点击运行后能展示任务、用户、分配关系和指标；
- 核心测试仍然通过；
- 控制台目标不被破坏。

风险点：

- Qt 路径和 CMake 配置可能需要适配本机安装目录；
- GUI 绘图坐标缩放需要单独处理；
- 不要把 Qt 类型引入核心算法层。

### 阶段 4：AIAgent 与实验报告生成

目标：支持自然语言生成实验配置，并输出实验摘要报告。

建议任务：

1. 新增 `agent` 模块：

```text
GeoTaskShield/agent/
  ExperimentAgent.h/.cpp
  RuleBasedConfigParser.h/.cpp
  ReportGenerator.h/.cpp
```

2. 第一版先做规则解析，不直接依赖在线 LLM：
   - “100 个用户，50 个任务，k=5，使用匈牙利算法”
   - “对比三种隐私机制”
3. 将自然语言转换为 `SimulationConfig` 和策略名称。
4. 基于 `EvaluationMetrics` 和 CSV 生成 Markdown 实验报告。
5. 后续再接入大模型 API。

阶段 4 验收标准：

- 输入自然语言后能生成可运行配置；
- 能调用现有仿真链路；
- 能生成 Markdown 报告；
- 没有网络依赖时仍有本地规则兜底。

风险点：

- LLM 接口不稳定，不应作为第一版唯一实现；
- 需要避免 Agent 直接操作 GUI 或绕过核心业务接口。

### 阶段 5：实验能力增强

目标：让系统更适合论文/课程项目展示。

建议任务：

1. 批量实验：
   - 多组 worker/task 数量；
   - 多组 epsilon；
   - 多组 k；
   - 多组 grid size。
2. 增加指标：
   - 用户负载均衡；
   - 算法公平性；
   - 隐私-效用比；
   - 任务超时率。
3. 增加图表导出：
   - CSV；
   - Markdown；
   - 后续 Qt Charts。
4. 完善测试：
   - 将当前单文件测试拆分为多测试目标；
   - 可考虑引入 GoogleTest 或 Catch2。

阶段 5 验收标准：

- 能一键跑批量实验；
- 能生成多算法、多隐私机制对比表；
- 结果可复现。

### 阶段 6：工程整理与发布

目标：形成更标准的 C++/Qt 项目交付形态。

建议任务：

1. 整理 README：
   - 项目介绍；
   - 构建说明；
   - 运行截图；
   - 模块说明。
2. 清理旧入口：
   - 评估是否删除或迁移 `GeoTaskShield.cpp` / `GeoTaskShield.h`。
3. 增加代码风格约束：
   - 命名规范；
   - clang-format；
   - 编译警告级别。
4. 增加安装/打包：
   - Qt 部署；
   - Windows 可执行程序打包。

---

## 8. 建议下一步

建议下一轮直接进入阶段 3：Qt GUI 可视化。

最小落地顺序：

1. 修改 CMake，增加 Qt Widgets 可选构建；
2. 创建 `gui/MainWindow` 和 Qt 应用入口；
3. 先显示参数面板和运行按钮；
4. 点击按钮调用当前 `SimulationEngine`；
5. 显示 `EvaluationMetrics`；
6. 再实现 `MapCanvas` 绘图。

这一顺序风险最低，因为当前核心仿真链路已经稳定，GUI 可以逐步接入而不改动算法层。
