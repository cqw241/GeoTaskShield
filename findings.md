# Findings & Decisions

## Requirements
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

## Issues Encountered
| Issue | Resolution |
|-------|------------|
| No existing planning files | Created `task_plan.md`, `findings.md`, and `progress.md` in project root. |
| Normal PowerShell did not expose the MSVC/Ninja build environment earlier | Used Visual Studio DevCmd for all verification commands. |
| Current project had no Git repository | Initialized Git, committed the Phase 2 baseline on `main`, and created `develop` plus `feature/phase3-qt-gui`. |
| PowerShell rejected `&&` | Use separate `shell_command` calls for sequential git operations in this environment. |
| GUI smoke test showed missing `Qt6Core.dll` dialog | Added CTest environment for Qt `PATH` and `QT_PLUGIN_PATH`; added `windeployqt` post-build deployment for the GUI executable. |

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
