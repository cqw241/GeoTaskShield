# Task Plan: GeoTaskShield Phase 2 - Algorithms and Privacy

## Goal
Complete Phase 2 of GeoTaskShield by adding additional privacy mechanisms, assignment algorithms, comparative experiment support, and verification while keeping the core logic Qt-independent.

## Current Phase
Complete

## Phases

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

## Notes
- The current project is not a git repository, so progress must be tracked in planning files and local file state.
- Existing build verification uses Visual Studio DevCmd because the normal PowerShell PATH does not expose `cl.exe`/`ninja`.
- User mentioned Qt-installed CMake/Ninja are available; if VS DevCmd is unavailable later, search Qt install paths for those tools.
