# Phase 4 AIAgent and Report Generation Design

## Scope
Phase 4 adds a local experiment agent that converts a small natural-language request into simulation settings, runs existing GeoTaskShield algorithms, and generates a Markdown experiment report. The first version is rule-based and offline by default. Online large-model calls are not required for this phase.

## Architecture
- `GeoTaskShieldCore` gains a new `agent` module.
- `RuleBasedConfigParser` extracts simulation counts, privacy parameters, selected privacy mechanism, selected assignment algorithm, and comparison intent.
- `ExperimentAgent` orchestrates parsing, strategy creation, `SimulationEngine`, and report generation.
- `ReportGenerator` formats deterministic Markdown from experiment results.
- Existing privacy, assignment, simulation, evaluation, and CSV modules remain unchanged.

## Data Flow
1. User provides a natural-language prompt such as `100 个用户，50 个任务，k=5，使用匈牙利算法`.
2. `RuleBasedConfigParser` returns an `ExperimentRequest`.
3. `ExperimentAgent` runs one or more simulations using factories and `SimulationEngine`.
4. `ReportGenerator` emits Markdown containing configuration, result table, and a short summary.

## Model API Boundary
The user provided an Aliyun Bailian API key and model name for future model calls. This phase does not need a model call. If a future feature uses it, the key must be read from an environment variable such as `DASHSCOPE_API_KEY`; secrets must not be written to source, docs, generated reports, or commits.

## Testing
Core tests cover parser behavior, agent execution, and Markdown report contents. Tests use real parser and simulation code without network access.
