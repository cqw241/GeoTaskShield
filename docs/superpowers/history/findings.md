# Findings & Decisions

## Requirements
- Enter Demo Readiness after Phase 12.
- Do not continue asynchronous LLM calls, timeout controls, complex error UX, Qt Graphs, or GoogleTest migration.
- Verify non-Qt Debug build/core tests and Qt Debug build/GUI smoke tests.
- Verify demo workflow for Simulation, Batch Results, Agent Assistant local provider, and optional DashScope entry/config docs.
- Prepare `v0.9.0` release hardening by updating version, README, HANDOFF, CHANGELOG, demo guide, and package script.
- Generate or confirm a runnable release package.
- Enter GeoTaskShield Phase 12: extend `IExperimentAssistant` with an optional real LLM provider.
- Keep the existing offline rule-based assistant as the default.
- Support an Aliyun Bailian / DashScope provider using environment variables for secrets and model selection.
- Configure the user's local environment for DashScope API access without writing the provided key into repository files.
- Do not require network access or a real API key in automated tests.
- Do not modify `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, `BatchExperiment`, or algorithm semantics.
- Keep Qt types confined to `GeoTaskShield/gui`.
- Enter GeoTaskShield Phase 11: Intelligent Experiment Assistant.
- Work from `develop` on `feature/phase11-intelligent-assistant`.
- Add an offline `Agent Assistant` GUI tab with natural-language input, Analyze, parsed intent preview, Markdown preview, and optional Markdown export.
- Add Qt-free assistant abstractions: `AssistantRequest`, `AssistantResponse`, `ExperimentIntent`, `IExperimentAssistant`, `RuleBasedAssistant`, and `MockLLMAssistant`.
- Parse workers, tasks, privacy, algorithm, metrics, and compare intent locally.
- Analyze current Batch Results CSV/filtered rows and generate Markdown conclusions for best `completionRate`, best `privacyUtilityRatio`, lowest `averagePrivacyLoss`, and best `fairnessIndex`.
- Provide local next-experiment suggestions.
- Do not connect to real online LLMs, store API keys, introduce network dependencies, change `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, or `BatchExperiment` semantics, add Qt Graphs, migrate GoogleTest, or add algorithms.
- Enter GeoTaskShield Phase 8: release and demo hardening for `v0.7.0`.
- Phase 8 must freeze feature scope and avoid Markdown preview, filtered export, Qt Graphs, and online LLM work.
- Release package must include `phase5_batch_results.csv` or equivalent demo data.
- Release package should include GUI demo and screenshot guidance for Simulation and Batch Results tabs.
- Enter GeoTaskShield Phase 7: GUI batch CSV result visualization.
- Phase 7 must only add CSV result analysis/display in the GUI.
- Do not change `SimulationEngine`, privacy/assignment factories, Agent, or `BatchExperiment` semantics.
- Store Qt-free batch result record/loading/model code outside `gui`, in the non-Qt core build.
- Support current Phase 5 snake_case CSV headers plus selected aliases for distance, runtime, and privacy-utility fields.
- Table sorting must be numeric for numeric fields.
- Enter GeoTaskShield Phase 5: experiment capability enhancement.
- Enter GeoTaskShield Phase 6: engineering cleanup and release.
- Prepare a Git Flow release from the completed Phase 1-5 implementation.
- Keep Phase 6 focused on engineering delivery, not new algorithm or GUI functionality.
- Add batch experiments across multiple worker/task counts, epsilon values, k values, and grid sizes.
- Add load balance, fairness, privacy-utility ratio, and timeout metrics.
- Add CSV and Markdown batch export.
- Preserve existing console, Qt GUI, Agent, and core tests.
- Enter GeoTaskShield Phase 4: AIAgent and experiment report generation.
- Use local rule-based parsing first; do not require online LLM calls for the first version.
- If Python becomes necessary, use `uv`.
- If model calls become necessary, use Aliyun Bailian with model `kimi-k2.5`, but do not commit API keys.
- Enter GeoTaskShield Phase 2: algorithms and privacy improvement.
- Add more privacy mechanisms beyond `GridPrivacy`.
- Add more assignment algorithms beyond `NearestGreedyAlgorithm`.
- Keep core logic C++/CMake based and do not connect Qt yet.
- Preserve the Phase 1 console MVP and make Phase 2 verifiable.

## Research Findings
- Current branch for Demo Readiness is `release/v0.9.0`.
- `v0.9.0` release hardening is scoped to packaging, documentation, demo workflow verification, and smoke-test coverage for the real demo CSV.
- Existing package script already includes `docs/demo`, `phase5_batch_results.csv`, and `phase5_batch_report.md`; updating its default version is sufficient for package naming.
- Current branch for Phase 12 is `feature/phase12-real-llm-provider`.
- Official Aliyun Bailian / Model Studio documentation describes an OpenAI-compatible Chat Completions endpoint.
- Beijing region default base URL is `https://dashscope.aliyuncs.com/compatible-mode/v1`; HTTP requests post to `/chat/completions`.
- The official examples read the bearer token from `DASHSCOPE_API_KEY`.
- Phase 12 should support `DASHSCOPE_MODEL` for the model name; the user requested `kimi-k2.5`.
- Official reference used: https://help.aliyun.com/zh/model-studio/qwen-api-via-openai-chat-completions
- Current branch for implementation is `feature/phase11-intelligent-assistant`.
- Phase 11 spec is `docs/superpowers/specs/2026-04-28-phase11-intelligent-experiment-assistant-design.md`.
- Current release documentation says `v0.8.0` already includes Batch Results Markdown preview/export and filtered CSV export.
- Existing `agent` module has `ExperimentRequest`, `RuleBasedConfigParser`, `ExperimentReport`, `ReportGenerator`, and `ExperimentAgent`.
- Existing `experiment` module has `BatchResultRecord`, `BatchResultCsvLoader`, and `BatchResultModel` with filter, summary, chart, Markdown, and CSV report support.
- Existing `gui` module has `BatchResultsWidget` and `MainWindow` with `Simulation` and `Batch Results` tabs.
- Current source root is `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield`.
- Current core modules:
  - `model`: `Location`, `Task`, `Worker`, `Assignment`, `ExperimentConfig`
  - `simulation`: `DataGenerator`, `SimulationEngine`
  - `privacy`: `IPrivacyMechanism`, `GridPrivacy`
  - `assignment`: `IAssignmentAlgorithm`, `NearestGreedyAlgorithm`
  - `evaluation`: `EvaluationMetrics`, `MetricsCalculator`
  - `app/console/main.cpp`: console MVP entry
- Current test file is `GeoTaskShield/tests/test_core.cpp`.
- Current build target `GeoTaskShield` is a console executable, not Qt.
- `IPrivacyMechanism::apply` is `const`, so new privacy mechanisms should avoid mutable global state or expose deterministic behavior through `PrivacyContext`/fixed local seeds.
- `AssignmentContext` already has alpha/beta/gamma/delta weights, suitable for `ScoreGreedyAlgorithm` without changing the interface.
- Existing core test is a single executable with simple `require` assertions; Phase 2 can extend it without adding a test framework dependency.
- Console entry currently prints one experiment result; Phase 2 should convert it to a compact comparison table.
- `EvaluationMetrics` already exposes all values needed for console comparison and CSV export.
- Current `phase5_batch_results.csv` header is `scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,completed_tasks,total_tasks,completion_rate,average_moving_distance,total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,fairness_index,privacy_utility_ratio,timeout_rate`.
- Phase 7 uses `experiment/BatchResultCsvLoader` and `experiment/BatchResultModel` as Qt-free display/data helpers.
- Phase 7 GUI uses `BatchResultsWidget` and `MetricBarChart` under `gui`.

## Technical Decisions
| Decision | Rationale |
|----------|-----------|
| Phase 11 adds assistant classes under `agent` | Assistant behavior is natural-language parsing and analysis, and must remain Qt-free. |
| Phase 11 GUI reads filtered records through `BatchResultsWidget` | The existing widget already owns current CSV/filter state; exposing records avoids scraping table text. |
| Phase 11 will use TDD against the existing lightweight test executables | The project has not migrated to GoogleTest and the user explicitly asked not to migrate it. |
| Phase 11 default assistant is `RuleBasedAssistant` | The phase is offline-first and must not call real online LLMs. |
| Phase 12 should add a real provider behind `IExperimentAssistant` instead of changing core assistant callers | The existing GUI and tests can preserve local behavior while exposing optional online analysis. |
| Phase 12 should use an injectable HTTP client | Automated tests must remain hermetic and must not depend on network access or real credentials. |
| DashScope provider configuration should use `DASHSCOPE_API_KEY`, `DASHSCOPE_MODEL`, and optional `DASHSCOPE_BASE_URL` | These names match official examples for the key and keep model/base URL runtime-configurable. |
| Demo Readiness should use `release/v0.9.0` | This work prepares a versioned release package rather than adding new feature behavior. |
| Optional DashScope provider is not a main demo dependency | The user explicitly asked to confirm only the entry and environment variable explanation. |
| Stage 2 will keep `SimulationEngine` strategy-based | Existing design already accepts privacy and assignment strategy objects. |
| Implement factories after concrete algorithms | Factory tests are clearer once there are multiple concrete strategies. |
| CSV export is a separate `data` module | Export should not pollute algorithm, simulation, or GUI layers. |
| Console comparison uses factories | The console app now exercises the same strategy creation path intended for future GUI controls. |
| CSV output path is `phase2_results.csv` in the project working directory | Simple deterministic output for Phase 2 verification and later report generation. |
| Phase 3 GUI should be an optional Qt Widgets target | This keeps non-Qt console/test builds working on machines without Qt while still supporting GUI builds where Qt is installed. |
| GUI should call `SimulationEngine` and factories only | This preserves the current strategy-based architecture and avoids duplicating algorithm logic in widgets. |
| `MapCanvas` should render from `SimulationRunResult` | The result contains generated workers/tasks, privacy-adjusted workers, assignments, and metrics needed for visualization. |
| Show assigned links using task IDs and worker IDs | `Assignment` stores identifiers rather than indices, so the canvas should build lookup maps before drawing. |
| CTest needs Qt runtime environment for GUI smoke tests | Without Qt `bin` on `PATH`, Windows opens a missing `Qt6Core.dll` dialog and blocks the test process. |
| GUI target should deploy Qt runtime after build on Windows | `windeployqt` makes the built GUI executable runnable from its output directory without manual DLL path setup. |
| Phase 4 should stay offline by default | The handoff explicitly recommends a rule-based first version and no single dependency on online LLMs. |
| API key handling must be runtime-only | The provided key is sensitive and should not be written into source, docs, reports, tests, or commits. |
| Agent module should call existing factories and `SimulationEngine` | This keeps the agent as orchestration/reporting and avoids duplicating algorithms. |
| MSVC should compile project sources as UTF-8 | Phase 4 tests and demo prompts include Chinese text, so `/utf-8` avoids code page 936 warnings and misread literals. |
| Phase 5 metrics should be computed in `MetricsCalculator` | Metrics already centralize assignment quality, and GUI/Agent can consume expanded `EvaluationMetrics` without new algorithm dependencies. |
| Batch experiments should use existing factories and `SimulationEngine` | This keeps batch execution aligned with console, GUI, and Agent behavior. |
| Batch export should live in a new `experiment` module | The existing `CsvExporter` is a Phase 2 summary exporter; batch rows need scenario configuration fields and Markdown export. |
| Phase 6 should use `release/phase6-engineering-release` | The task is release preparation, and `git_guide.md` includes `release/*` for this purpose. |
| Phase 6 should tag `v0.6.0` after verification | There are no existing tags, and the phase number gives a clear first release identifier. |
| Packaging should be script-based and generated under `out/package` | This keeps Qt DLLs and ZIP artifacts out of Git while making local delivery repeatable. |
| Phase 7 puts batch CSV analysis in `experiment` | The loader/model are Qt-free and need non-Qt test coverage. |
| Phase 7 avoids Qt Charts | Qt Charts is deprecated in Qt 6.11 and a custom single-metric bar chart is enough for this phase. |
| Summary cards include source identity | Metric values alone are not enough to identify which scenario/privacy/algorithm produced the best result. |
| Phase 8 uses `release/v0.7.0` | The user requested a release branch to stabilize Phase 7 outputs into a versioned package. |
| Phase 8 keeps `phase5_batch_results.csv` as demo data | It is the current real batch CSV output and already matches the Batch Results loader. |

## Issues Encountered
| Issue | Resolution |
|-------|------------|
| No existing planning files | Created `task_plan.md`, `findings.md`, and `progress.md` in project root. |
| Normal PowerShell did not expose the MSVC/Ninja build environment earlier | Used Visual Studio DevCmd for all verification commands. |
| Current project had no Git repository | Initialized Git, committed the Phase 2 baseline on `main`, and created `develop` plus `feature/phase3-qt-gui`. |
| PowerShell rejected `&&` | Use separate `shell_command` calls for sequential git operations in this environment. |
| GUI smoke test showed missing `Qt6Core.dll` dialog | Added CTest environment for Qt `PATH` and `QT_PLUGIN_PATH`; added `windeployqt` post-build deployment for the GUI executable. |
| Phase 3 was complete on a feature branch | Merged it into `develop`, then created `feature/phase4-ai-agent-report` for Phase 4 per Git Flow. |
| Secret scan found no committed API key text | Searched project files excluding `.git`, `out`, and `.vs`; no provided key string was found. |
| Phase 5 started from `develop` after syncing `main` publish notes | Merged `main` into `develop`, then created `feature/phase5-experiment-enhancements`. |
| Phase 6 started after Phase 5 feature completion | Merged `feature/phase5-experiment-enhancements` into `develop`, then created `release/phase6-engineering-release`. |
| Phase 7 started from `develop` | Created `feature/phase7-gui-batch-results`; implementation is GUI-only CSV analysis. |
| Phase 8 started from `develop` | Created `release/v0.7.0`; no new runtime features are in scope. |

## Resources
- Project specification: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield.md`
- Core CMake file: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield\CMakeLists.txt`
- Current core test: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield\tests\test_core.cpp`
- Phase 2 CSV output: `D:\VS2026_Projects\GeoTaskShield\phase2_results.csv`

## Visual/Browser Findings
- No browser or image inspection used.

## Interview Architecture Briefing Findings
- User goal: prepare for an interview for Professor Tao Peng's project group, with emphasis on undergraduate C++ development, GUI/system implementation in phase one, and LLM/Agent development in phase two.
- Current repository state: project has an existing `v0.9.0`-level history covering C++ core simulation, Qt GUI, batch experiments, Markdown/CSV reporting, offline Agent Assistant, and optional DashScope/OpenAI-compatible provider.
- Existing planning files already contain a detailed phase history from core MVP through demo readiness; current task is explanation/preparation, not business-code modification.
- `git status` shows this briefing updated `task_plan.md` and `progress.md`; there are also pre-existing untracked `deliverables/` and a Chinese PDF resume file.
- Reading README/HANDOFF from the current PowerShell session produced mojibake for Chinese text, but module names, commands, paths, and phase structure were still recoverable from the files and existing planning records.
- `rg --files` failed with `Access is denied`; use PowerShell `Get-ChildItem` as the fallback for source-tree inspection.
- README confirms the project positioning: a C++20/CMake/Qt Widgets simulator for privacy-preserving task allocation in mobile crowdsensing.
- Phase-one capabilities map directly to the recruiting note: C++ core implementation, algorithm modules, Qt Widgets GUI, CMake/CTest, Windows packaging, CSV/Markdown outputs.
- Phase-two capabilities are already scaffolded: `agent` includes local rule-based parsing/reporting, GUI `Agent Assistant`, and an optional DashScope/OpenAI-compatible provider behind `IExperimentAssistant`.
- Source layout confirms clean module boundaries: pure C++ core modules (`model`, `simulation`, `privacy`, `assignment`, `evaluation`, `data`, `experiment`, `agent`) and Qt-only UI code under `gui`.
- `SimulationEngine` is the main orchestrator: generate workers/tasks, apply privacy mechanism, run assignment algorithm, then calculate evaluation metrics.
- Privacy and assignment extension points are interface-based: `IPrivacyMechanism` / `PrivacyFactory` and `IAssignmentAlgorithm` / `AssignmentAlgorithmFactory`.
- Current privacy mechanisms: Grid, K-Anonymity, Laplace Noise. Current assignment algorithms: Nearest Greedy, Score Greedy, Hungarian.
- Metrics include completion rate, moving distance, reward, privacy loss, runtime, load balance, Jain fairness, privacy-utility ratio, and timeout rate.
- Privacy implementations preserve the core experiment idea: `GridPrivacy` maps real coordinates to grid-cell centers, `KAnonymityPrivacy` exposes local centroids, and `LaplaceNoisePrivacy` adds deterministic seeded Laplace coordinate noise for reproducible experiments.
- Assignment implementations cover increasing complexity: nearest-distance greedy, weighted score greedy, and Hungarian minimum-cost matching over expanded worker slots.
- `MetricsCalculator` evaluates completed assignments against real worker positions, so privacy perturbs decision input while metrics still capture actual movement cost and timeout.
- `BatchExperimentRunner` and exporters turn single simulations into scenario matrices and CSV/Markdown outputs.
- `BatchResultModel` supports filtered records, sorted records, best-metric summaries, chart bars, Markdown reports, and CSV reports for the GUI Batch Results tab.
- `RuleBasedAssistant` analyzes user prompts and current batch rows locally; `OpenAICompatibleAssistant` wraps that local result and optionally sends prompt + batch rows to an OpenAI-compatible provider when environment variables are configured.
- `MainWindow` wires three tabs: `Simulation`, `Batch Results`, and `Agent Assistant`; the assistant receives current filtered batch rows through a provider callback rather than scraping UI table text.
- Professor context from public pages: ?? is listed as a Guangzhou University Computer Science and Cyber Engineering associate professor; public profile research interests include mobile crowdsensing networks, social-network privacy protection, cloud computing security, and blockchain technology. This aligns strongly with GeoTaskShield's mobile crowdsensing/privacy-preserving task-allocation theme.

---
*Update this file after every 2 view/browser/search operations*
*This prevents visual information from being lost*
