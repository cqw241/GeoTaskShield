# GeoTaskShield 项目交接文档

更新时间：2026-04-27
项目路径：`D:\VS2026_Projects\GeoTaskShield`
当前状态：阶段 1、阶段 2、阶段 3、阶段 4、阶段 5 已完成并通过验收。阶段 6 尚未开始。
当前开发分支：`feature/phase5-experiment-enhancements`

---

## 1. 项目概述

GeoTaskShield 是一个面向移动群智感知场景的隐私保护任务分配仿真系统。当前版本使用 C++20/CMake 实现核心仿真、控制台实验、批量实验、Qt Widgets GUI 可视化，以及本地规则型 AIAgent 实验报告生成能力。

系统当前支持：

- 生成模拟任务与移动用户；
- 对用户位置进行隐私保护；
- 使用不同任务分配算法完成任务匹配；
- 计算完成率、平均真实移动距离、总收益、隐私损失、算法运行时间、负载均衡、公平性、隐私效用比和超时率；
- 在控制台输出算法对比结果；
- 导出 CSV 实验结果；
- 通过 Qt Widgets GUI 选择参数、运行仿真、显示指标和二维分配图；
- 通过本地规则型 AIAgent 解析自然语言实验请求并生成 Markdown 报告；
- 一键运行批量实验并导出 CSV/Markdown 报告。

---

## 2. Git 状态与工作流

项目已初始化为 Git 仓库，并按 `git_guide.md` 使用 Git Flow。

分支状态：

| 分支 | 说明 |
|---|---|
| `main` | 已发布阶段 4 项目状态，远端默认分支 |
| `develop` | 已合入阶段 4，并同步 GitHub 发布记录 |
| `feature/phase3-qt-gui` | 阶段 3 功能分支，提交 `3b66cbf feat(gui): add Qt Widgets simulation UI` |
| `feature/phase4-ai-agent-report` | 阶段 4 功能分支 |
| `feature/phase5-experiment-enhancements` | 当前阶段 5 功能分支 |

后续 Git 操作要求：

- 新功能必须从 `develop` 创建 `feature/*` 分支；
- 提交信息遵守 `<type>(<scope>): <description>`；
- 不要把构建产物、Qt 部署 DLL、`.vs/`、`out/` 提交进仓库；
- 不要把 API key 或其他密钥写入源码、文档、报告或提交。

---

## 3. 当前工程结构

```text
GeoTaskShield/
  .gitignore
  CMakeLists.txt
  CMakePresets.json
  GeoTaskShield.md
  HANDOFF.md
  git_guide.md
  task_plan.md
  findings.md
  progress.md
  phase2_results.csv
  docs/
    superpowers/
      specs/
      plans/
  GeoTaskShield/
    CMakeLists.txt
    app/
      console/main.cpp
      agent_demo/main.cpp
      batch_demo/main.cpp
    model/
    simulation/
    privacy/
    assignment/
    evaluation/
    data/
    agent/
    experiment/
    gui/
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
| `agent` | 自然语言实验请求解析、实验编排、Markdown 报告生成 |
| `gui` | Qt Widgets 桌面 GUI |
| `app/console` | 控制台实验入口 |
| `app/agent_demo` | AIAgent 报告生成 demo 入口 |
| `tests` | 当前核心测试入口 |

---

## 4. 阶段 1 完成情况

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

---

## 5. 阶段 2 完成情况

阶段 2 目标：完善隐私机制、分配算法、对比实验能力和结果导出能力。

新增隐私机制：

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

新增分配算法：

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

控制台程序运行 3 x 3 对比矩阵：

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

CSV 输出：

```text
D:\VS2026_Projects\GeoTaskShield\phase2_results.csv
```

当前 CSV 包含 1 行表头和 9 行实验结果。

---

## 6. 阶段 3 完成情况

阶段 3 目标：把当前控制台实验能力接入 Qt Widgets 桌面界面。

已完成内容：

- CMake 增加可选 Qt GUI 构建开关：
  - `GEOTASKSHIELD_BUILD_GUI`
- 新增 Qt preset：
  - `x64-debug-qt`
  - 当前本机 Qt 路径：`D:/Qt/6.11.0/msvc2022_64`
- 新增 GUI 目标：
  - `GeoTaskShieldGui`
- 新增 GUI smoke test：
  - `GeoTaskShieldGuiSmokeTests`
- 新增 GUI 文件：

```text
GeoTaskShield/gui/
  app/main.cpp
  MainWindow.h/.cpp
  MapCanvas.h/.cpp
  ParameterPanel.h/.cpp
  ResultPanel.h/.cpp
  LogPanel.h/.cpp
  tests/test_gui_smoke.cpp
```

GUI 当前能力：

- 可启动 Qt Widgets 主窗口；
- 左侧参数面板可设置 worker 数量、task 数量、grid size、k、epsilon、隐私机制、分配算法；
- 点击运行后调用 `PrivacyFactory`、`AssignmentAlgorithmFactory` 和 `SimulationEngine`；
- 中间 `MapCanvas` 绘制 worker、暴露位置、task 和 assignment 虚线；
- 右侧显示完成任务数、完成率、平均真实移动距离、总收益、平均隐私损失、算法耗时；
- 底部显示运行日志。

Windows Qt 部署：

- `GeoTaskShieldGui` 构建后自动调用 `windeployqt`；
- 运行目录为：

```text
D:\VS2026_Projects\GeoTaskShield\out\build\x64-debug-qt\GeoTaskShield\GeoTaskShieldGui.exe
```

注意：

- `GeoTaskShield.exe` 和 `GeoTaskShieldTests.exe` 不是 Qt 程序，不需要 `windeployqt`；
- 只有 `GeoTaskShieldGui.exe` 和 GUI smoke test 需要 Qt 运行时；
- CTest 已为 GUI smoke test 注入 Qt runtime 路径，避免缺失 `Qt6Core.dll` 弹窗阻塞。

---

## 7. 阶段 4 完成情况

阶段 4 目标：支持自然语言生成实验配置，并输出实验摘要报告。

已完成内容：

```text
GeoTaskShield/agent/
  ExperimentRequest.h
  RuleBasedConfigParser.h/.cpp
  ExperimentReport.h
  ReportGenerator.h/.cpp
  ExperimentAgent.h/.cpp
```

新增 demo：

```text
GeoTaskShield/app/agent_demo/main.cpp
```

新增可执行程序：

```text
GeoTaskShieldAgentDemo
```

当前 AIAgent 能力：

- 使用本地规则解析自然语言请求；
- 支持中英文基础表达；
- 可解析：
  - worker 数量；
  - task 数量；
  - `k`；
  - `epsilon`；
  - `grid size`；
  - 隐私机制；
  - 分配算法；
  - “对比三种隐私机制”类型请求；
- 调用现有 `PrivacyFactory`、`AssignmentAlgorithmFactory` 和 `SimulationEngine`；
- 生成 Markdown 实验报告。

示例输入：

```text
100 个用户，50 个任务，k=5，使用匈牙利算法
80 workers, 40 tasks, epsilon=0.5, use laplace and score greedy
对比三种隐私机制，50个用户，20个任务，使用最近贪心
```

报告包含：

- 原始请求；
- 实验配置；
- Markdown 指标表；
- 最佳完成率摘要。

模型 API 说明：

- 阶段 4 第一版按交接计划使用本地规则解析，不依赖在线 LLM；
- 用户曾提供阿里云百炼 API key 和模型名 `kimi-k2.5`，但当前实现未调用大模型；
- API key 未写入源码、文档、测试、报告或提交；
- 后续如接入百炼，应通过运行时环境变量读取，例如 `DASHSCOPE_API_KEY`，不要硬编码。

---

## 8. 构建与验收方式

推荐在 Visual Studio Developer Command Prompt 或通过 `VsDevCmd.bat` 加载 MSVC 环境后执行。

### 8.1 非 Qt 构建、测试、控制台实验

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure && out\build\x64-debug\GeoTaskShield\GeoTaskShield.exe'
```

验收结果：

```text
1/1 Test #1: GeoTaskShieldCoreTests Passed
100% tests passed, 0 tests failed out of 1
```

### 8.2 Agent demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe
```

默认会输出 Markdown 实验报告。

也可以传入自定义请求：

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe "对比三种隐私机制，30个用户，10个任务，使用匈牙利算法"
```

### 8.3 Batch demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldBatchDemo.exe
```

默认会输出批量实验 Markdown 报告，并生成：

```text
phase5_batch_results.csv
phase5_batch_report.md
```

### 8.4 Qt GUI 构建与测试

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

验收结果：

```text
2/2 tests passed
```

运行 GUI：

```powershell
out\build\x64-debug-qt\GeoTaskShield\GeoTaskShieldGui.exe
```

Qt 相关注意：

- `x64-debug-qt` preset 当前写入本机 Qt 路径 `D:/Qt/6.11.0/msvc2022_64`；
- 如果换机器，需要修改 `CMAKE_PREFIX_PATH`；
- 构建时可能提示 `Could NOT find WrapVulkanHeaders`，当前 Widgets GUI 不依赖 Vulkan，已验证不影响构建和测试。

---

## 9. 当前设计约束与注意事项

1. 核心算法层保持 Qt 无关。
   - `model`、`simulation`、`privacy`、`assignment`、`evaluation`、`data`、`agent` 均为纯 C++。
   - Qt 类型只应出现在 `gui` 模块。

2. 当前测试仍是轻量自定义测试。
   - 文件：`GeoTaskShield/tests/test_core.cpp`
   - GUI smoke test：`GeoTaskShield/gui/tests/test_gui_smoke.cpp`
   - 未引入 GoogleTest/Catch2。

3. Hungarian 算法当前语义：
   - 支持一对一任务匹配；
   - 通过展开 `Worker::maxTasks` 为多个 worker slot 支持简单容量；
   - 不支持一个任务需要多个 worker 的严格优化建模。

4. Laplace 噪声当前语义：
   - 使用固定随机种子，保证测试和实验可复现；
   - 属于仿真意义上的坐标扰动，不是完整差分隐私证明实现。

5. AIAgent 当前语义：
   - 本地规则解析；
   - 不依赖网络；
   - 不调用大模型；
   - 不保存密钥。

6. 批量实验当前语义：
   - 使用预设场景列表；
   - 通过现有工厂和 `SimulationEngine` 执行；
   - CSV 包含算法运行耗时，重复运行时该列可能有轻微波动。

7. `GeoTaskShield.cpp` 和 `GeoTaskShield.h` 仍保留在目录中，但当前 CMake 不使用它们作为入口。

8. MSVC 构建已给 `GeoTaskShieldCore` 增加 `/utf-8`，用于稳定支持中文 prompt 测试和 demo 字符串。

---

## 10. 后续开发计划

### 阶段 5：实验能力增强

目标：让系统更适合论文/课程项目展示。

已完成内容：

- 新增扩展指标：
  - `userLoadStdDev`：worker 任务负载标准差，越低表示越均衡；
  - `fairnessIndex`：基于 worker 任务负载的 Jain 公平性指数；
  - `privacyUtilityRatio`：`completionRate / (1 + averagePrivacyLoss)`；
  - `timeoutRate`：已分配任务中真实移动时间超过 deadline 的比例。
- 新增批量实验模块：

```text
GeoTaskShield/experiment/
  BatchExperiment.h/.cpp
  BatchExperimentExporter.h/.cpp
```

- 新增批量实验 demo：

```text
GeoTaskShield/app/batch_demo/main.cpp
```

- 新增可执行程序：

```text
GeoTaskShieldBatchDemo
```

批量实验当前覆盖：

- 多组 worker/task 数量；
- 多组 epsilon；
- 多组 k；
- 多组 grid size；
- 多组隐私机制和分配算法组合。

批量实验输出：

```text
phase5_batch_results.csv
phase5_batch_report.md
```

阶段 5 验收结果：

- 可一键运行批量实验；
- 可生成多算法、多隐私机制、多参数对比表；
- 可生成报告展示用 CSV/Markdown；
- 核心测试通过；
- 结果由固定随机种子驱动，除算法运行耗时外可复现。

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

## 11. 建议下一步

建议下一轮进入阶段 6：工程整理与发布。

最低风险顺序：

1. 清理或迁移旧入口 `GeoTaskShield.cpp` / `GeoTaskShield.h`；
2. 拆分当前单文件核心测试，或引入 GoogleTest/Catch2；
3. 增加 `.clang-format` 和基础代码风格约束；
4. 整理 README 截图、模块图和阶段 5 输出示例；
5. 评估 Qt 部署脚本和 Windows 打包流程；
6. 后续再考虑 Qt Charts 或 GUI 报告面板。

这一顺序优先降低工程交付风险，不需要先改动算法核心。
