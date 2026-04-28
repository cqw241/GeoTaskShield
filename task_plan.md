# Task Plan: GeoTaskShield Phase 11 - Intelligent Experiment Assistant

## Goal
Implement Phase 11 as an offline intelligent experiment assistant with a Qt-free assistant abstraction, local rule-based analysis over current Batch Results rows, and a Qt Widgets `Agent Assistant` tab.

## Current Phase
Phase 11 complete on `feature/phase11-intelligent-assistant`; ready for review/merge

## Phase 11 Success Criteria
- Work happens on `feature/phase11-intelligent-assistant`.
- Add an `Agent Assistant` GUI tab with natural-language input, Analyze action, parsed intent preview, Markdown analysis preview, and Markdown export.
- Add Qt-free assistant types under `GeoTaskShield/agent`: `AssistantRequest`, `AssistantResponse`, `ExperimentIntent`, `IExperimentAssistant`, `RuleBasedAssistant`, and `MockLLMAssistant`.
- Parse workers, tasks, privacy, algorithm, metric, and compare intent locally.
- Analyze current Batch Results CSV/filtered rows without rerunning experiments.
- Markdown conclusions include best `completionRate`, best `privacyUtilityRatio`, lowest `averagePrivacyLoss`, and best `fairnessIndex`.
- Provide local next-experiment suggestions.
- Do not connect to real online LLMs, store API keys, introduce network dependencies, modify `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, or `BatchExperiment` semantics, add Qt Graphs, migrate GoogleTest, or add algorithms.
- Non-Qt core tests and Qt GUI smoke tests pass.

## Phase 8 Success Criteria
- Work happens on `release/v0.7.0`.
- Do not add Markdown report preview, filtered-result export, Qt Graphs, online LLM, or new runtime features.
- Update project version, README, CHANGELOG, HANDOFF, demo guide, and package script for `v0.7.0`.
- Package includes `phase5_batch_results.csv` and `docs/demo/v0.7.0-gui-demo-guide.md`.
- Verify non-Qt Debug build/tests, Qt Debug build/tests, Qt Release build, and Windows ZIP packaging.
- Merge release into `main`, tag `v0.7.0`, merge back into `develop`, and push branches/tags.

## Phase 7 Success Criteria
- Work happens on `feature/phase7-gui-batch-results`.
- Add Qt-free batch result loading/model classes under `GeoTaskShield/experiment`.
- Keep Qt types isolated to `GeoTaskShield/gui` widgets.
- Load current `phase5_batch_results.csv` and same-structure CSV files.
- Support privacy/algorithm filtering, metric selection, summary cards, single-metric bar chart, detail table, and numeric sorting.
- Summary cards include both metric values and `scenario + privacy + algorithm` source context.
- Do not change `SimulationEngine`, factories, Agent, or `BatchExperiment` semantics.
- Non-Qt and Qt CTest targets pass.

## Phase 6 Success Criteria
- Phase 5 is merged into `develop`.
- Release work happens on `release/phase6-engineering-release`.
- Remove unused `GeoTaskShield.cpp` / `GeoTaskShield.h` template files.
- Add `.clang-format`, warning options, a Qt Release preset, and a Windows packaging helper.
- Update README, HANDOFF, planning files, and release notes.
- Verify non-Qt Debug, Qt Debug, Release Qt, package script, secret scan, and diff checks.
- Merge release into `main`, tag `v0.6.0`, merge back into `develop`, and push branches/tags.

## Phase 5 Success Criteria
- Work happens on `feature/phase5-experiment-enhancements`.
- Add batch experiment scenarios and runner in a Qt-independent core module.
- Add user load balance, fairness, privacy-utility ratio, and timeout metrics.
- Generate batch CSV and Markdown outputs from a single demo executable.
- Existing console, Agent, GUI, and tests continue to pass.
- Update README and HANDOFF with Phase 5 status and commands.

## Phase 4 Success Criteria
- Work happens on `feature/phase4-ai-agent-report` after integrating completed Phase 3 into `develop`.
- Add `GeoTaskShield/agent` without introducing Qt dependencies into the core.
- Parse simple Chinese/English natural-language experiment prompts into runnable configuration.
- Run current simulation pipeline through factories and `SimulationEngine`.
- Generate Markdown experiment reports.
- Do not commit API keys or require network access for first-version behavior.
- Existing CTest targets pass.

## Phase 3 Success Criteria
- Git repository follows `git_guide.md`: `main`, `develop`, and `feature/phase3-qt-gui` branches exist.
- GUI code is isolated from core algorithm modules.
- CMake can still build the existing console executable and tests without Qt GUI enabled.
- When Qt is available, a Qt Widgets executable can launch, accept simulation parameters, run `SimulationEngine`, display metrics, and draw workers, tasks, and assignments.
- Existing `GeoTaskShieldCoreTests` still pass.

## Phases

### Phase 0: Git Initialization
- [x] Initialize Git repository
- [x] Add `.gitignore` for generated build and IDE files
- [x] Commit current Phase 2 baseline on `main`
- [x] Create `develop`
- [x] Create `feature/phase3-qt-gui`
- **Status:** complete

### Phase 1: Baseline Review and Scope Lock
- [x] Confirm current Phase 1 MVP structure
- [x] Identify existing core modules and tests
- [x] Define Phase 2 implementation scope
- **Status:** complete

### Phase 2: Privacy Mechanisms
- [x] Add `KAnonymityPrivacy`
- [x] Add `LaplaceNoisePrivacy`
- [x] Add tests for exposed locations and privacy loss behavior
- [x] Keep `IPrivacyMechanism` interface unchanged unless strictly required
- **Status:** complete

### Phase 3: Assignment Algorithms
- [x] Add `ScoreGreedyAlgorithm`
- [x] Add `HungarianAlgorithm` for one-worker-per-task matching
- [x] Add tests for deterministic worker selection and capacity handling
- [x] Keep `IAssignmentAlgorithm` interface unchanged unless strictly required
- **Status:** complete

### Phase 4: Factories and Console Comparison
- [x] Add privacy mechanism factory
- [x] Add assignment algorithm factory
- [x] Update console MVP to run a small comparison matrix
- [x] Print metrics for each privacy/algorithm combination
- **Status:** complete

### Phase 5: Result Export
- [x] Add CSV exporter for experiment summary rows
- [x] Export algorithm, privacy mechanism, completion rate, distance, reward, privacy loss, runtime
- [x] Keep export code outside GUI concerns
- **Status:** complete

### Phase 6: Verification and Cleanup
- [x] Run CMake configure/build
- [x] Run CTest
- [x] Run console MVP and inspect output
- [x] Update planning files with final status and test results
- **Status:** complete

### Phase 7: Qt GUI Build Integration
- [x] Add optional Qt Widgets build path in CMake
- [x] Keep console and tests independent from Qt
- [x] Add GUI target sources under `GeoTaskShield/gui`
- **Status:** complete

### Phase 8: GUI Components
- [x] Add `MainWindow`
- [x] Add `ParameterPanel`
- [x] Add `ResultPanel`
- [x] Add `LogPanel`
- [x] Add `MapCanvas`
- **Status:** complete

### Phase 9: GUI Simulation Flow
- [x] Convert panel inputs into `SimulationConfig`
- [x] Create privacy and assignment strategies through factories
- [x] Run `SimulationEngine` from the GUI
- [x] Display metrics and draw workers/tasks/assignments
- **Status:** complete

### Phase 10: Verification and Commit
- [x] Run CMake configure/build
- [x] Run CTest
- [x] Build GUI target if Qt is available
- [x] Commit feature work using commit convention
- **Status:** complete

### Phase 11: Phase 4 Branch and Design
- [x] Merge completed `feature/phase3-qt-gui` into `develop`
- [x] Create `feature/phase4-ai-agent-report`
- [x] Write Phase 4 design document
- [x] Write Phase 4 implementation plan
- **Status:** complete

### Phase 12: Agent Parser
- [x] Add parser tests
- [x] Add `ExperimentRequest`
- [x] Add `RuleBasedConfigParser`
- **Status:** complete

### Phase 13: Report Generation
- [x] Add report generator tests
- [x] Add report data structures
- [x] Add Markdown generation
- **Status:** complete

### Phase 14: Experiment Agent
- [x] Add agent orchestration tests
- [x] Add `ExperimentAgent`
- [x] Support single-run and privacy comparison prompts
- **Status:** complete

### Phase 15: Demo and Verification
- [x] Add CMake wiring
- [x] Add `GeoTaskShieldAgentDemo`
- [x] Run build and CTest
- [x] Run report demo
- [x] Commit Phase 4 feature
- **Status:** complete

### Phase 16: Phase 5 Branch and Design
- [x] Sync latest `main` publish record back into `develop`
- [x] Create `feature/phase5-experiment-enhancements`
- [x] Write Phase 5 design document
- [x] Write Phase 5 implementation plan
- **Status:** complete

### Phase 17: Expanded Metrics
- [x] Add failing tests for new metrics
- [x] Add new `EvaluationMetrics` fields
- [x] Update `MetricsCalculator`
- **Status:** complete

### Phase 18: Batch Experiments
- [x] Add batch runner tests
- [x] Add `experiment` module
- [x] Add batch runner CMake wiring
- **Status:** complete

### Phase 19: Batch Export and Demo
- [x] Add batch exporter tests
- [x] Add CSV/Markdown batch exporter
- [x] Add `GeoTaskShieldBatchDemo`
- **Status:** complete

### Phase 20: Documentation, Verification, Push
- [x] Update README and HANDOFF
- [x] Run non-Qt verification
- [x] Run Qt verification
- [x] Commit and push feature branch
- **Status:** complete

### Phase 21: Phase 6 Release Branch
- [x] Merge Phase 5 feature branch into `develop`
- [x] Create `release/phase6-engineering-release`
- [x] Write Phase 6 design document
- [x] Write Phase 6 implementation plan
- **Status:** complete

### Phase 22: Engineering Cleanup
- [x] Remove unused Visual Studio template entry files
- [x] Add code formatting convention
- [x] Add compiler warning convention
- [x] Add Qt Release preset
- **Status:** complete

### Phase 23: Packaging and Release Docs
- [x] Add Windows packaging helper
- [x] Add changelog/release notes
- [x] Update README and HANDOFF for Phase 6
- **Status:** complete

### Phase 24: Verification and Publishing
- [x] Run non-Qt Debug verification
- [x] Run Qt Debug verification
- [x] Run Release Qt build verification
- [x] Run CMake install verification
- [x] Run package script
- [x] Run diff and secret checks
- [x] Commit release branch
- [x] Merge to `main`, tag `v0.6.0`, merge back to `develop`, and push
- **Status:** complete

### Phase 25: Phase 7 Design and Plan
- [x] Create `feature/phase7-gui-batch-results`
- [x] Write Phase 7 design document
- [x] Incorporate review feedback about file boundaries, CSV aliases, numeric sorting, and chart labels
- [x] Write Phase 7 implementation plan
- **Status:** complete

### Phase 26: Batch Result Data Layer
- [x] Add failing tests for CSV loading and result modeling
- [x] Add `BatchResultRecord`
- [x] Add `BatchResultCsvLoader`
- [x] Add `BatchResultModel`
- [x] Wire data layer into `GeoTaskShieldCore`
- [x] Run non-Qt core tests
- **Status:** complete

### Phase 27: Batch Results GUI
- [x] Add failing GUI smoke test expectations
- [x] Add `MetricBarChart`
- [x] Add `BatchResultsWidget`
- [x] Integrate `Batch Results` tab into `MainWindow`
- [x] Verify existing Simulation tab still runs
- [x] Run Qt GUI smoke test
- **Status:** complete

### Phase 28: Phase 7 Verification and Documentation
- [x] Run full non-Qt Debug verification
- [x] Run full Qt Debug verification
- [x] Update README, HANDOFF, planning files, and progress log
- **Status:** complete

### Phase 29: Phase 8 Release Branch and Docs
- [x] Create `release/v0.7.0` from `develop`
- [x] Update version metadata to `0.7.0` / `v0.7.0`
- [x] Update README with Batch Results demo usage and screenshot guidance
- [x] Add `docs/demo/v0.7.0-gui-demo-guide.md`
- [x] Update CHANGELOG and HANDOFF
- **Status:** complete

### Phase 30: Phase 8 Package Hardening
- [x] Update Windows package script default output to `v0.7.0`
- [x] Include `docs/demo` in the release package
- [x] Keep `phase5_batch_results.csv` in the release package
- [x] Run release verification and package checks
- [x] Publish `v0.7.0`
- **Status:** complete

### Phase 31: Phase 11 Planning and Red Tests
- [x] Create `feature/phase11-intelligent-assistant` from `develop`
- [x] Write Phase 11 design spec
- [x] Update planning files for Phase 11
- [x] Add failing non-Qt assistant tests
- [x] Add failing Qt GUI smoke expectations for `Agent Assistant`
- **Status:** complete

### Phase 32: Qt-Free Assistant Core
- [x] Add assistant request/response/intent/interface headers
- [x] Implement `RuleBasedAssistant`
- [x] Implement deterministic local `MockLLMAssistant`
- [x] Wire assistant sources into `GeoTaskShieldCore`
- [x] Run non-Qt tests
- **Status:** complete

### Phase 33: Agent Assistant GUI
- [x] Add `AgentAssistantWidget`
- [x] Expose current filtered Batch Results rows from `BatchResultsWidget`
- [x] Integrate `Agent Assistant` tab into `MainWindow`
- [x] Add smoke-test helpers for assistant analysis/export
- [x] Run Qt GUI smoke tests
- **Status:** complete

### Phase 34: Verification and Documentation
- [x] Run full non-Qt Debug verification
- [x] Run full Qt Debug verification
- [x] Run diff check and secret/API-key scan
- [x] Update README/HANDOFF if implementation changes user-facing workflow
- [x] Update planning files with final status
- **Status:** complete

## Key Questions
1. Should Hungarian matching support tasks requiring multiple workers in Phase 2?
   - Decision: no. Phase 2 implements one-worker-per-task matching and leaves multi-worker assignment for a later extension.
2. Should Laplace privacy use a full differential privacy proof?
   - Decision: no. Phase 2 implements coordinate-level Laplace noise as a simulator mechanism with documented privacy/utility tradeoff.
3. Should console comparison require user input?
   - Decision: no. Keep Phase 2 console deterministic with hard-coded comparison settings.

## Decisions Made
| Decision | Rationale |
|----------|-----------|
| Keep core modules independent from Qt | Current architecture intentionally makes GUI a later layer and keeps algorithms testable from console. |
| Use TDD for new mechanisms and algorithms | New behavior is algorithmic and easy to regress; tests define expected deterministic behavior. |
| Implement Hungarian for square/rectangular one-to-one assignment only | It is sufficient for Phase 2 comparison and avoids overcomplicating multi-worker task semantics. |
| Use deterministic random seeds in tests | Privacy noise and data generation must be reproducible for reliable verification. |
| Phase 6 uses a `release/*` branch | The work is delivery hardening and Git Flow explicitly reserves release branches for release preparation. |
| Tag Phase 6 as `v0.6.0` | The project has no prior tags; using the phase number keeps the first release tag unambiguous. |
| Phase 7 stores CSV analysis in `experiment` rather than `gui` | The data/model layer is Qt-free and belongs in the non-Qt core build; only widgets live in `gui`. |
| Phase 7 uses custom chart painting, not Qt Charts | Qt Charts is deprecated in Qt 6.11 and this phase only needs a single-metric bar chart. |
| Phase 8 freezes features for release hardening | The user explicitly deferred report preview, filtered export, Qt Graphs, and online LLM work to Phase 9. |
| Phase 11 stays offline and Qt-free outside GUI | The user explicitly requires no real LLM, no API keys, no network dependency, and Qt types only in `gui`. |
| Phase 11 analyzes current Batch Results rows | This reuses the existing CSV/filter pipeline and avoids changing simulation or batch experiment semantics. |
| `MockLLMAssistant` is deterministic and local | It proves the interface boundary without introducing online model behavior. |

## Errors Encountered
| Error | Attempt | Resolution |
|-------|---------|------------|
| No planning files existed | 1 | Created `task_plan.md`, `findings.md`, and `progress.md`. |
| Missing new headers during TDD red builds | 1 | Implemented each missing module after confirming the intended failure. |
| PowerShell rejected `&&` command separator | 1 | Ran `git add` and `git commit` as separate commands. |
| GUI smoke test hung behind Windows missing Qt6Core.dll dialog | 1 | Added Qt runtime/plugin environment for CTest and `windeployqt` deployment for GUI target. |
| PowerShell `Select-String -Recurse` unsupported | 1 | Used `Get-ChildItem -Recurse -File | Select-String` for secret scanning. |
| Missing `experiment/BatchExperiment.h` during Phase 5 red build | 1 | Implemented the batch experiment module and exporter after confirming the expected failure. |
| PowerShell `Select-Object -Index 32..42` treated the range as a string | 1 | Re-ran with a `$lines[32..42]` array slice. |
| MSVC warned that `/W4` overrides default `/W3` | 1 | Removed default `/W*` CMake flags before applying project warning options. |
| Missing `agent/MockLLMAssistant.h` during Phase 11 red build | 1 | Expected red test; implement Qt-free assistant headers and sources next. |
| Non-Qt Phase 11 green verification timed out after 124 seconds | 1 | Retry with a longer timeout before changing code. |
| Non-Qt Phase 11 green verification opened a Debug Runtime dialog | 2 | Killed the stale test process and fixed the assistant test fixture before rerunning. |

## Notes
- Existing build verification uses Visual Studio DevCmd because the normal PowerShell PATH does not expose `cl.exe`/`ninja`.
- User mentioned Qt-installed CMake/Ninja are available; if VS DevCmd is unavailable later, search Qt install paths for those tools.
- As of Phase 3, the project is a Git repository. Follow `git_guide.md` for all future git operations.
- Phase 5 batch outputs are generated by `GeoTaskShieldBatchDemo` into `phase5_batch_results.csv` and `phase5_batch_report.md`.
- Phase 6 package output is generated under `out/package` and must not be committed.
