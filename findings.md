# Findings & Decisions

## Requirements
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

## Resources
- Project specification: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield.md`
- Core CMake file: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield\CMakeLists.txt`
- Current core test: `D:\VS2026_Projects\GeoTaskShield\GeoTaskShield\tests\test_core.cpp`
- Phase 2 CSV output: `D:\VS2026_Projects\GeoTaskShield\phase2_results.csv`

## Visual/Browser Findings
- No browser or image inspection used.

---
*Update this file after every 2 view/browser/search operations*
*This prevents visual information from being lost*
