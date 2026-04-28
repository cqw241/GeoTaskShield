# GeoTaskShield 项目交接文档

更新时间：2026-04-28
项目路径：`D:\VS2026_Projects\GeoTaskShield`
当前状态：阶段 1 至阶段 12 已完成并通过本地验收，Demo Readiness 已固化为 `v0.9.0` 发布包。Phase 13 正在强化真实 LLM provider 的超时、错误回退和 GUI 非阻塞体验。
当前开发分支：`feature/phase13-agent-provider-hardening`

---

## 1. 项目概述

GeoTaskShield 是一个面向移动群智感知场景的隐私保护任务分配仿真系统。当前版本使用 C++20/CMake 实现核心仿真、控制台实验、批量实验、Qt Widgets GUI 可视化、本地规则型 AIAgent 实验报告生成能力，以及离线智能实验助手能力。

系统当前支持：

- 生成模拟任务与移动用户；
- 对用户位置进行隐私保护；
- 使用不同任务分配算法完成任务匹配；
- 计算完成率、平均真实移动距离、总收益、隐私损失、算法运行时间、负载均衡、公平性、隐私效用比和超时率；
- 在控制台输出算法对比结果；
- 导出 CSV 实验结果；
- 通过 Qt Widgets GUI 选择参数、运行仿真、显示指标和二维分配图；
- 通过 Qt Widgets GUI 的 `Batch Results` 页加载批量实验 CSV、筛选、排序、查看摘要卡片和单指标柱状图；
- 通过 Qt Widgets GUI 的 `Batch Results` 页导出当前筛选后的 CSV 结果；
- 通过 Qt Widgets GUI 的 `Batch Results` 页基于当前筛选结果预览或导出 Markdown 报告；
- 通过 Qt Widgets GUI 的 `Agent Assistant` 页输入自然语言请求，预览结构化实验意图，并基于当前 Batch Results 筛选结果生成或导出 Markdown 分析；
- 通过本地规则型 AIAgent 解析自然语言实验请求并生成 Markdown 报告；
- 通过 Qt-free `IExperimentAssistant` 抽象提供本地规则型分析助手和确定性 Mock LLM 助手；
- 一键运行批量实验并导出 CSV/Markdown 报告。

---

## 2. Git 状态与工作流

项目已初始化为 Git 仓库，并按 `git_guide.md` 使用 Git Flow。

分支状态：

| 分支 | 说明 |
|---|---|
| `main` | `v0.9.0` 发布基线，远端默认分支；包含发布后的 README 补充和 Simulation map legend |
| `develop` | 本地已快进同步到 `main`，作为下一阶段 `feature/*` 起点 |
| `feature/phase3-qt-gui` | 阶段 3 功能分支，提交 `3b66cbf feat(gui): add Qt Widgets simulation UI` |
| `feature/phase4-ai-agent-report` | 阶段 4 功能分支 |
| `feature/phase5-experiment-enhancements` | 阶段 5 功能分支，已合入 `develop` |
| `feature/phase9-markdown-report-gui` | 阶段 9 功能分支，已合入 `develop` |
| `feature/phase11-intelligent-assistant` | 阶段 11 智能实验助手功能分支 |
| `feature/phase12-real-llm-provider` | 阶段 12 可选真实 LLM provider 功能分支 |
| `release/phase6-engineering-release` | 阶段 6 发布准备分支 |
| `release/v0.7.0` | 阶段 8 / `v0.7.0` 发布准备分支 |
| `release/v0.8.0` | 阶段 10 / `v0.8.0` 发布准备分支 |
| `release/v0.9.0` | Demo Readiness / `v0.9.0` 发布准备分支，已完成 |

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
    demo/
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

## 8. 阶段 5：实验能力增强

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

## 9. 阶段 6：工程整理与发布

目标：形成更标准的 C++/Qt 项目交付形态。

已完成内容：

1. 整理 README：
   - 项目介绍；
   - 构建说明；
   - Release 构建与打包说明；
   - 模块说明。
2. 清理旧入口：
   - 删除未被 CMake 使用的 `GeoTaskShield.cpp` / `GeoTaskShield.h`。
3. 增加代码风格约束：
   - `.clang-format`；
   - MSVC `/W4`；
   - 非 MSVC `-Wall -Wextra -Wpedantic`。
4. 增加安装/打包：
   - 新增 `x64-release-qt` preset；
   - 新增 `scripts/package_windows.ps1`；
   - 打包 console、agent demo、batch demo、Qt GUI、Qt runtime 和示例报告。
5. 发布版本：
   - 版本号：`v0.6.0`；
   - 发布准备分支：`release/phase6-engineering-release`；
   - 合入 `main`，打 tag，并回合到 `develop`。

阶段 6 验收结果：

- 非 Qt Debug 构建和核心测试通过；
- Qt Debug 构建、GUI smoke test 通过；
- Qt Release 构建和测试通过；
- CMake install 规则通过；
- Windows ZIP 打包脚本通过；
- `v0.6.0` 发布包生成于 `out\package\GeoTaskShield-v0.6.0-windows-x64.zip`；
- 发布包属于生成产物，不提交进 Git。

## 10. 阶段 7：GUI 数据可视化增强

目标：在不改变核心算法、Agent 或 BatchExperiment 语义的前提下，为 Qt GUI 增加批量实验 CSV 结果分析与展示页。

已完成内容：

- 新增 Qt-free 批量结果展示层：

```text
GeoTaskShield/experiment/
  BatchResultRecord.h
  BatchResultCsvLoader.h/.cpp
  BatchResultModel.h/.cpp
```

- 新增 Qt Widgets 批量结果页面：

```text
GeoTaskShield/gui/
  BatchResultsWidget.h/.cpp
  MetricBarChart.h/.cpp
```

- `MainWindow` 改为标签页结构：
  - `Simulation`：保留原有单次仿真页面；
  - `Batch Results`：加载和分析批量实验 CSV。
- `BatchResultCsvLoader` 支持：
  - 当前 `phase5_batch_results.csv` snake_case 表头；
  - 常见字段 alias，如 `average_true_distance`、`runtimeMs`、`privacyUtilityRatio`；
  - UTF-8 BOM；
  - CRLF/LF；
  - quoted fields；
  - 缺列和非法数字的明确错误信息。
- `BatchResultModel` 支持：
  - 按 privacy/algorithm 筛选；
  - 按字段类型排序；
  - 计算最佳 `completionRate`、最佳 `privacyUtilityRatio`、最佳 `fairnessIndex`、最低 `averagePrivacyLoss`；
  - 生成 `scenario | privacy | algorithm` 标签的柱状图数据。
- `BatchResultsWidget` 支持：
  - 打开 `phase5_batch_results.csv` 或同结构 CSV；
  - 左侧加载和筛选控件；
  - 摘要卡片显示指标值和来源实验配置；
  - 自绘单指标柱状图；
  - 当前行详情；
  - 可排序结果表格，数值列按数值排序。

阶段 7 验收结果：

- 非 Qt Debug 构建和核心测试通过；
- Qt Debug 构建和 GUI smoke test 通过；
- 未引入 Qt Charts；
- 未修改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory`、Agent 或 `BatchExperiment` 语义。

## 11. 阶段 8：Release and Demo Hardening

目标：将阶段 7 的 GUI 批量结果可视化成果固化为 `v0.7.0` 发布版，而不是继续新增功能。

已完成内容：

1. 更新版本号：
   - CMake project version：`0.7.0`；
   - Windows 打包默认版本：`v0.7.0`。
2. 更新 README：
   - Batch Results 页功能说明；
   - GUI demo 使用流程；
   - 示例 CSV 表头；
   - Simulation tab 和 Batch Results tab 截图说明。
3. 新增 demo 指南：

```text
docs/demo/v0.7.0-gui-demo-guide.md
```

4. 更新 release 文档：
   - `CHANGELOG.md` 增加 `v0.7.0`；
   - `HANDOFF.md` 更新阶段 8、版本和后续方向。
5. 更新 Windows 打包脚本：
   - 默认输出 `GeoTaskShield-v0.7.0-windows-x64.zip`；
   - release 包继续包含 `phase5_batch_results.csv`；
   - release 包新增 `docs/demo/` demo 指南。

阶段 8 验收结果：

- 非 Qt Debug 构建和核心测试通过；
- Qt Debug 构建和 GUI smoke test 通过；
- Qt Release 构建通过；
- Windows ZIP 打包脚本通过；
- `v0.7.0` 发布包生成于 `out\package\GeoTaskShield-v0.7.0-windows-x64.zip`；
- 发布包包含 `phase5_batch_results.csv` 和 `docs/demo/v0.7.0-gui-demo-guide.md`；
- 未新增 Markdown 报告预览、筛选结果导出、Qt Graphs 或在线 LLM。

## 12. 阶段 9：GUI 报告与筛选结果导出入口

目标：在不重新运行批量实验、不引入 Qt WebEngine/Qt Charts、且不改变核心仿真语义的前提下，为 `Batch Results` 页增加 Markdown 报告预览/导出入口，以及当前筛选 CSV 导出入口。

已完成内容：

- `BatchResultModel` 新增 `markdownReport()`，基于当前 privacy/algorithm 筛选后的记录生成 Markdown 表格与摘要；
- `BatchResultModel` 新增 `csvReport()`，基于当前 privacy/algorithm 筛选后的记录生成 Phase 5 同结构 CSV；
- `BatchResultsWidget` 左侧控制区新增：
  - `Export Filtered CSV`：通过保存对话框导出当前筛选后的 `.csv` 文件；
  - `Preview Markdown`：用 Qt `QTextEdit::setMarkdown` 预览当前报告；
  - `Export Markdown`：通过保存对话框导出 `.md` 文件；
- 核心测试和 GUI smoke test 覆盖：
  - 当前筛选 CSV 只包含匹配筛选条件的行；
  - CSV 导出入口存在并可写出文件；
  - Markdown 预览/导出入口存在；
  - 加载 CSV 后可生成 Markdown 标题、表格行和摘要；
  - 导出的 `.md` 文件包含生成的报告内容。

设计约束：

- CSV 和 Markdown 内容来自已加载 CSV 的当前筛选结果；
- 不调用在线 LLM；
- 不修改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory`、Agent 或 `BatchExperiment` 语义；
- Qt 类型仍只出现在 `gui` 模块，Markdown 报告字符串生成保持在 Qt-free `experiment` 模块。
- 导出 CSV 反映当前 privacy/algorithm 筛选状态，不重新解释表格列头排序。

## 13. 阶段 10：Release and Demo Hardening

目标：将阶段 9 的 GUI Markdown 报告预览/导出和当前筛选 CSV 导出能力固化为 `v0.8.0` 正式发布版，不继续新增 Qt Graphs、GoogleTest 迁移或在线 LLM 集成。

已完成内容：

1. 更新版本号：
   - CMake project version：`0.8.0`；
   - Windows 打包默认版本：`v0.8.0`。
2. 更新 README：
   - Current release 改为 `v0.8.0`；
   - Batch Results 页补充 `Export Filtered CSV`、`Preview Markdown`、`Export Markdown` 流程；
   - Release 包路径改为 `out/package/GeoTaskShield-v0.8.0-windows-x64.zip`。
3. 新增 demo 指南：

```text
docs/demo/v0.8.0-gui-demo-guide.md
```

4. 更新 release 文档：
   - `CHANGELOG.md` 增加 `v0.8.0`；
   - `HANDOFF.md` 更新阶段 10、版本和后续方向。
5. 更新 Windows 打包脚本：
   - 默认输出 `GeoTaskShield-v0.8.0-windows-x64.zip`；
   - release 包继续包含 `phase5_batch_results.csv`、`phase5_batch_report.md` 和 `docs/demo/` demo 指南。

阶段 10 验收结果：

- 非 Qt Debug 构建和核心测试通过；
- Qt Debug 构建和 GUI smoke test 通过；
- Qt Release 构建通过；
- Windows ZIP 打包脚本通过；
- `v0.8.0` 发布包生成于 `out\package\GeoTaskShield-v0.8.0-windows-x64.zip`；
- 发布包包含 `docs/demo/v0.8.0-gui-demo-guide.md`；
- 未新增 Qt Graphs，未迁移 GoogleTest，未接入在线 LLM。

---

## 14. 阶段 11：Intelligent Experiment Assistant

目标：在不接入真实在线 LLM、不保存 API key、不改变核心仿真语义的前提下，为 GeoTaskShield 增加离线智能实验助手能力。第一版重点是 Agent 架构扩展、本地智能分析和 GUI 自然语言入口。

已完成内容：

- 新增 Qt-free assistant 抽象和实现：

```text
GeoTaskShield/agent/
  AssistantRequest.h
  AssistantResponse.h
  ExperimentIntent.h
  IExperimentAssistant.h
  RuleBasedAssistant.h/.cpp
  MockLLMAssistant.h/.cpp
```

- `RuleBasedAssistant` 支持解析：
  - worker 数量；
  - task 数量；
  - privacy 机制；
  - assignment algorithm；
  - metric 名称；
  - compare intent。
- `RuleBasedAssistant` 可基于当前 Batch Results 筛选记录生成 Markdown 分析，包括：
  - best `completionRate`；
  - best `privacyUtilityRatio`；
  - lowest `averagePrivacyLoss`；
  - best `fairnessIndex`；
  - 后续实验建议。
- `MockLLMAssistant` 作为确定性本地 mock，实现 `IExperimentAssistant`，用于验证未来 LLM 接入边界，不读取 API key、不访问网络。
- 新增 Qt Widgets 页面：

```text
GeoTaskShield/gui/
  AgentAssistantWidget.h/.cpp
```

- `MainWindow` 新增第三个 tab：
  - `Simulation`
  - `Batch Results`
  - `Agent Assistant`
- `Agent Assistant` tab 支持：
  - 自然语言输入；
  - `Analyze`；
  - parsed intent preview；
  - Markdown analysis preview；
  - `Export Markdown`。
- `BatchResultsWidget` 新增只读方法暴露当前筛选记录，供助手分析使用；不解析表格文本，不重新运行实验。
- 核心测试和 GUI smoke test 覆盖：
  - assistant intent parsing；
  - 四个关键指标分析；
  - 空数据提示；
  - deterministic mock assistant；
  - `Agent Assistant` tab 存在；
  - GUI 输入 prompt 后可生成 intent 和 Markdown；
  - Markdown 导出。

设计约束：

- 不调用真实在线 LLM；
- 不引入 API key；
- 不引入网络依赖；
- 不修改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory`、`BatchExperiment` 语义；
- 不新增算法；
- 不引入 Qt Graphs 或 Qt Charts；
- 不迁移 GoogleTest/Catch2；
- Qt 类型仍只出现在 `gui` 模块。

阶段 11 验收结果：

- 非 Qt Debug 构建和核心测试通过；
- Qt Debug 构建和 GUI smoke test 通过；
- `Agent Assistant` tab 存在；
- 输入示例 prompt 后可以生成结构化 intent 和 Markdown 分析；
- 无 API key、无网络依赖。

---

## 14.5. Phase 12: Optional Real LLM Provider

目标：在不改变核心仿真语义、不保存 API key、默认仍可离线运行的前提下，将 `IExperimentAssistant` 扩展为可选真实 LLM provider。

当前实现范围：

- 新增 Qt-free provider 抽象和实现：
  - `agent/HttpClient.h`
  - `agent/WinHttpClient.h/.cpp`
  - `agent/OpenAICompatibleAssistant.h/.cpp`
- `OpenAICompatibleAssistant` 先用 `RuleBasedAssistant` 生成本地 intent preview，再将用户 prompt、当前 Batch Results 行和本地 intent 发送给 OpenAI-compatible Chat Completions provider。
- GUI `Agent Assistant` tab 新增 provider selector：
  - `Local rule-based` 默认选项，不需要网络或 API key；
  - `Aliyun Bailian (DashScope)` 可选项，只有用户显式选择后才会调用真实 provider。
- DashScope 配置使用运行时环境变量：
  - `DASHSCOPE_API_KEY`：必需，真实 provider 缺失时 fail closed，不发起网络请求；
  - `DASHSCOPE_MODEL`：可选，默认 `kimi-k2.5`；
  - `DASHSCOPE_BASE_URL`：可选，默认 `https://dashscope.aliyuncs.com/compatible-mode/v1`；
  - `DASHSCOPE_TIMEOUT_MS`：可选，默认 `15000` 毫秒。
- 自动化测试通过 fake HTTP transport 验证请求体、鉴权 header 和响应解析，不依赖网络或真实 API key。

约束：

- 不要把 API key 写入源码、文档、测试、报告或提交历史。
- 不修改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory`、`BatchExperiment` 或算法语义。
- Qt 类型仍只允许出现在 `gui` 模块。
- Phase 12 第一版真实 provider 调用为同步请求；Phase 13 已在 GUI 层引入后台线程、状态提示和超时配置。

---

## 14.6. Demo Readiness / v0.9.0 Release Hardening

目标：把 Phase 11/12 的智能实验助手能力固化为可演示、可打包的 `v0.9.0`，不继续扩展异步调用、超时控制、复杂错误提示、Qt Graphs 或 GoogleTest 迁移。

本轮发布范围：

- 项目版本更新为 `0.9.0`。
- Windows 打包默认版本更新为 `v0.9.0`。
- 新增 `docs/demo/v0.9.0-gui-demo-guide.md`，覆盖：
  - `Simulation` tab 默认仿真；
  - `Batch Results` 加载 `phase5_batch_results.csv`、筛选、排序、导出 CSV、预览/导出 Markdown；
  - `Agent Assistant` 默认 `Local rule-based` 分析和导出 Markdown；
  - `Aliyun Bailian (DashScope)` provider 仅作为可选入口和环境变量说明，不作为主演示依赖。
- GUI smoke test 增强为直接加载仓库根目录的 `phase5_batch_results.csv` 并验证导出路径。
- 发布包继续包含 demo CSV、batch report、README/HANDOFF/CHANGELOG 和 `docs/demo`。

保持不变：

- 不新增算法；
- 不改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory` 或 `BatchExperiment` 语义；
- 不新增 Qt Graphs；
- 不迁移 GoogleTest；
- 不把 API key 写入仓库。

---

## 14.7. Phase 13: Agent Provider Hardening

目标：在不改变核心仿真、隐私机制、分配算法、批量实验语义且不保存 API key 的前提下，让可选真实 LLM provider 更适合真实 GUI 使用。

当前实现范围：

- `HttpRequest` 增加 `timeoutMs`，`OpenAICompatibleAssistant` 通过 `DASHSCOPE_TIMEOUT_MS` 或默认配置传递 provider 请求超时。
- `WinHttpClient` 在发送请求前应用 WinHTTP 超时设置。
- `OpenAICompatibleAssistant` 保持先运行本地 `RuleBasedAssistant`，provider 缺少密钥、请求失败、超时、空内容或非预期响应时返回本地分析 fallback。
- GUI `Agent Assistant` 的 DashScope provider 路径改为后台线程执行，避免真实网络请求阻塞 Qt UI。
- GUI 新增 provider 状态文本；运行中禁用 provider 选择和 Analyze 按钮，完成后显示成功或 unavailable/fallback 状态。
- 自动化测试仍使用 fake HTTP transport 和缺 key fallback，不依赖真实 API key 或网络访问。

保持不变：

- `Local rule-based` 仍是默认 provider。
- 不新增算法，不修改 `SimulationEngine`、`PrivacyFactory`、`AssignmentAlgorithmFactory` 或 `BatchExperiment`。
- API key 只允许来自运行时环境变量，不写入源码、文档、测试、报告或提交历史。

---

## 15. 构建与验收方式

推荐在 Visual Studio Developer Command Prompt 或通过 `VsDevCmd.bat` 加载 MSVC 环境后执行。

### 15.1 非 Qt 构建、测试、控制台实验

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure && out\build\x64-debug\GeoTaskShield\GeoTaskShield.exe'
```

验收结果：

```text
1/1 Test #1: GeoTaskShieldCoreTests Passed
100% tests passed, 0 tests failed out of 1
```

### 15.2 Agent demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe
```

默认会输出 Markdown 实验报告。

也可以传入自定义请求：

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe "对比三种隐私机制，30个用户，10个任务，使用匈牙利算法"
```

### 15.3 Batch demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldBatchDemo.exe
```

默认会输出批量实验 Markdown 报告，并生成：

```text
phase5_batch_results.csv
phase5_batch_report.md
```

### 15.4 Qt GUI 构建与测试

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

### 15.5 Release 构建与 Windows 打包

Release Qt 构建：

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-release-qt && cmake --build out\build\x64-release-qt'
```

Windows ZIP 打包：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_windows.ps1
```

默认输出：

```text
out\package\GeoTaskShield-v0.9.0-windows-x64.zip
```

打包输出位于 `out/`，属于生成产物，不提交进 Git。

---

## 16. 当前设计约束与注意事项

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

7. 旧 Visual Studio 模板入口 `GeoTaskShield.cpp` 和 `GeoTaskShield.h` 已在阶段 6 删除，当前入口统一位于 `GeoTaskShield/app/*/main.cpp`。

8. MSVC 构建已给项目目标增加 `/utf-8`，用于稳定支持中文 prompt 测试和 demo 字符串。

9. 阶段 6 增加 `.clang-format` 作为 C++ 格式约束，并给 MSVC 目标增加 `/W4` 警告等级。

---

## 17. 建议下一步

Phase 13 稳定后可继续考虑：

1. 拆分当前较大的 `GeoTaskShield/tests/test_core.cpp`，或评估是否迁移到 GoogleTest/Catch2；
2. 支持多轮实验助手上下文和更细粒度的实验参数建议；
3. 扩展更多隐私保护机制和任务分配算法；
4. 如后续需要论文级图表或复杂交互分析，再评估 Qt Graphs / Qt Charts；当前自绘柱状图足够支撑 `v0.9.0` 演示。
