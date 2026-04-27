# GeoTaskShield

GeoTaskShield is a C++20/CMake simulator for privacy-preserving task allocation in mobile crowdsensing. It models workers and sensing tasks on a 2D map, applies location privacy mechanisms, runs task assignment algorithms, evaluates utility/privacy metrics, and presents results through console, CSV, Qt Widgets GUI, and a local experiment-report agent.

## Current Status

Completed phases:

- Phase 1: Console MVP simulation pipeline.
- Phase 2: Additional privacy mechanisms, assignment algorithms, comparison experiments, and CSV export.
- Phase 3: Qt Widgets GUI visualization.
- Phase 4: Local rule-based AIAgent and Markdown experiment report generation.

Planned next phase:

- Phase 5: Batch experiment support, additional metrics, and richer report/chart export.

## Features

- Synthetic worker/task generation.
- Location privacy mechanisms:
  - Grid Privacy
  - K-Anonymity Privacy
  - Laplace Noise Privacy
- Assignment algorithms:
  - Nearest Greedy
  - Score Greedy
  - Hungarian
- Metrics:
  - completed task count
  - completion rate
  - average true moving distance
  - total reward
  - average privacy loss
  - algorithm runtime
- Console 3 x 3 privacy/algorithm comparison.
- CSV export to `phase2_results.csv`.
- Qt Widgets GUI with parameter panel, map canvas, result panel, and log panel.
- Local natural-language experiment agent.
- Markdown experiment report generation.

## Repository Layout

```text
GeoTaskShield/
  CMakeLists.txt
  CMakePresets.json
  HANDOFF.md
  README.md
  git_guide.md
  phase2_results.csv
  GeoTaskShield/
    app/
      console/
      agent_demo/
    model/
    simulation/
    privacy/
    assignment/
    evaluation/
    data/
    agent/
    gui/
    tests/
```

## Requirements

- Windows
- Visual Studio 2026 with MSVC toolchain
- CMake
- Ninja
- Qt 6.11 MSVC kit for GUI builds

The current Qt preset expects:

```text
D:/Qt/6.11.0/msvc2022_64
```

If Qt is installed elsewhere, update `CMAKE_PREFIX_PATH` in `CMakePresets.json`.

## Build and Test

Run commands from the project root in a Visual Studio Developer Command Prompt, or call `VsDevCmd.bat` from PowerShell.

### Core Console Build

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure'
```

Run the console comparison:

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShield.exe
```

This prints 9 comparison rows and writes:

```text
phase2_results.csv
```

### Agent Report Demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe
```

With a custom request:

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe "对比三种隐私机制，30个用户，10个任务，使用匈牙利算法"
```

Example supported prompts:

```text
100 个用户，50 个任务，k=5，使用匈牙利算法
80 workers, 40 tasks, epsilon=0.5, use laplace and score greedy
对比三种隐私机制，50个用户，20个任务，使用最近贪心
```

### Qt GUI Build

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

Run the GUI:

```powershell
out\build\x64-debug-qt\GeoTaskShield\GeoTaskShieldGui.exe
```

`GeoTaskShieldGui` runs `windeployqt` after build on Windows. Non-Qt executables such as `GeoTaskShield.exe` and `GeoTaskShieldTests.exe` do not need `windeployqt`.

## AIAgent Notes

The current agent is intentionally local and rule-based. It does not call an online model and does not require network access.

If a future version integrates Aliyun Bailian or another model provider, API keys must be read from runtime environment variables such as `DASHSCOPE_API_KEY`. Do not commit API keys, generated secrets, or model credentials.

## Git Workflow

This project follows the Git Flow rules in `git_guide.md`:

- `main`: production-ready code
- `develop`: integration branch
- `feature/*`: feature work
- `hotfix/*`: urgent fixes
- `release/*`: release preparation

Commit messages use:

```text
<type>(<scope>): <description>
```

Examples:

```text
feat(agent): add experiment report agent
docs(handoff): update phase 4 status
```

## Documentation

- `HANDOFF.md`: current project handoff, phase status, verification commands, and next steps.
- `GeoTaskShield.md`: original project concept.
- `docs/superpowers/specs/`: design notes for implemented phases.
- `docs/superpowers/plans/`: implementation plans.

## Current Limitations

- The core tests use a lightweight custom assertion style rather than GoogleTest or Catch2.
- Hungarian assignment supports one task per expanded worker slot, not strict multi-worker task optimization.
- Laplace privacy is a simulator-level coordinate perturbation, not a full formal differential privacy proof.
- The AIAgent parser is rule-based and supports common prompt patterns only.
