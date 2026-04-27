# Task Plan: GeoTaskShield Phase 4 - AIAgent and Experiment Reports

## Goal
Add a local natural-language experiment agent and Markdown report generation while preserving the existing console, GUI, and core algorithm behavior.

## Current Phase
Phase 4 complete

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

## Errors Encountered
| Error | Attempt | Resolution |
|-------|---------|------------|
| No planning files existed | 1 | Created `task_plan.md`, `findings.md`, and `progress.md`. |
| Missing new headers during TDD red builds | 1 | Implemented each missing module after confirming the intended failure. |
| PowerShell rejected `&&` command separator | 1 | Ran `git add` and `git commit` as separate commands. |
| GUI smoke test hung behind Windows missing Qt6Core.dll dialog | 1 | Added Qt runtime/plugin environment for CTest and `windeployqt` deployment for GUI target. |
| PowerShell `Select-String -Recurse` unsupported | 1 | Used `Get-ChildItem -Recurse -File | Select-String` for secret scanning. |

## Notes
- The current project is not a git repository, so progress must be tracked in planning files and local file state.
- Existing build verification uses Visual Studio DevCmd because the normal PowerShell PATH does not expose `cl.exe`/`ninja`.
- User mentioned Qt-installed CMake/Ninja are available; if VS DevCmd is unavailable later, search Qt install paths for those tools.
- As of Phase 3, the project is a Git repository. Follow `git_guide.md` for all future git operations.
