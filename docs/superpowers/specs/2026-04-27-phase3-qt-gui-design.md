# Phase 3 Qt Widgets GUI Design

## Scope
Phase 3 adds a desktop GUI around the existing simulation pipeline. The GUI is a presentation layer only: it creates `SimulationConfig`, uses the existing factories to create strategies, calls `SimulationEngine`, and displays the returned `SimulationRunResult`.

## Architecture
- `GeoTaskShieldCore` remains Qt-independent.
- `GeoTaskShield` remains the console executable.
- `GeoTaskShieldGui` is a new Qt Widgets executable built only when Qt Widgets is found.
- GUI source files live under `GeoTaskShield/gui`.

## Components
- `MainWindow`: owns the layout and coordinates the run action.
- `ParameterPanel`: exposes worker count, task count, random seed, grid size, k, epsilon, privacy mechanism, and assignment algorithm.
- `ResultPanel`: displays completed tasks, completion rate, average true moving distance, total reward, average privacy loss, and algorithm runtime.
- `LogPanel`: shows concise run status messages.
- `MapCanvas`: draws real worker points, exposed worker points, task points, and assignment links.

## Data Flow
1. User edits parameters.
2. `MainWindow` reads `ParameterPanel`.
3. `PrivacyFactory` and `AssignmentAlgorithmFactory` create strategies.
4. `SimulationEngine::run` returns `SimulationRunResult`.
5. `ResultPanel`, `LogPanel`, and `MapCanvas` render the result.

## Error Handling
If a factory returns null, the GUI logs the problem and does not run. Numeric inputs use Qt widgets with constrained ranges so invalid values are prevented at input time.

## Testing
Existing core tests continue to verify algorithm behavior. Phase 3 adds a lightweight GUI smoke test target that constructs the main widgets offscreen when Qt is available. Full verification still includes configure, build, CTest, and GUI target build when Qt is installed.
