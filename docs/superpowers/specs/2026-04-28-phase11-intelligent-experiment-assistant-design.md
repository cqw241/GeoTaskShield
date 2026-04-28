# Phase 11 Design: Intelligent Experiment Assistant

## Goal

Phase 11 adds an offline intelligent experiment assistant to GeoTaskShield. The first version focuses on agent architecture extension, deterministic local intent parsing, local analysis over already loaded batch results, and a Qt GUI natural-language entry point.

This phase must not connect to a real online LLM, store API keys, introduce network dependencies, change core simulation semantics, add algorithms, migrate the test framework, or introduce Qt Graphs.

## Current Context

GeoTaskShield `v0.8.0` already provides:

- A Qt-free core with simulation, privacy, assignment, evaluation, data, agent, and experiment modules.
- A rule-based Phase 4 `agent` module that parses natural-language experiment requests and can run existing simulations through existing factories.
- A Phase 7/9 `experiment` module that loads batch CSV results, filters rows, computes summary records, exports filtered CSV, and generates Markdown.
- A Qt Widgets GUI with `Simulation` and `Batch Results` tabs.
- A `Batch Results` tab that can load CSV, filter by privacy and algorithm, preview Markdown, and export Markdown/CSV.

Phase 11 should reuse these boundaries. The assistant is an analysis/reporting layer over existing results and parsed intent, not a new simulation engine and not a new assignment or privacy feature.

## Branch

Development for this phase should happen from `develop` on:

```text
feature/phase11-intelligent-assistant
```

## Assumptions

- "Current Batch Results CSV/filtered results" means the assistant should analyze the rows currently visible under the `Batch Results` tab's active privacy and algorithm filters.
- If no batch CSV has been loaded, the assistant should still parse and preview the natural-language intent, then return a Markdown message explaining that no batch results are available for metric analysis.
- The first GUI version does not need a chat history or multi-turn memory.
- `MockLLMAssistant` is a deterministic local implementation for interface testing and demo purposes. It must not read environment variables, API keys, config files, or network endpoints.
- Qt types remain limited to `GeoTaskShield/gui/`. Assistant request/response models must use standard C++ types.

## Approaches Considered

Recommended approach: add a dedicated `Agent Assistant` tab backed by a Qt-free assistant interface. The tab can read the current filtered batch records from the existing `Batch Results` tab, pass them to a local assistant, show structured intent, and preview Markdown. This keeps the user workflow clear and preserves the existing Batch Results UI.

Alternative: add a small assistant panel directly inside `Batch Results`. This is less code, but it crowds an already dense tab and makes the natural-language assistant feel like a secondary export control instead of a distinct workflow.

Rejected for Phase 11: let the assistant run new simulations from prompts. That would blur the boundary between result analysis and experiment execution, and it risks changing or expanding `SimulationEngine`, factories, or `BatchExperiment` behavior. It can be revisited after the offline analysis assistant is stable.

## Scope

In scope:

- Add a third GUI tab named `Agent Assistant`.
- Provide a natural-language input box.
- Provide an `Analyze` button.
- Preview the parsed experiment intent in a structured form.
- Preview Markdown analysis output.
- Optionally export the generated Markdown.
- Add Qt-free assistant abstractions:
  - `AssistantRequest`
  - `AssistantResponse`
  - `ExperimentIntent`
  - `IExperimentAssistant`
  - `RuleBasedAssistant`
  - `MockLLMAssistant`
- Parse natural-language references to:
  - worker count
  - task count
  - privacy mechanism or privacy comparison
  - assignment algorithm
  - metric names
  - compare intent
- Generate local natural-language analysis from the current batch result records.
- Include Markdown conclusions for:
  - best `completionRate`
  - best `privacyUtilityRatio`
  - lowest `averagePrivacyLoss`
  - best `fairnessIndex`
- Generate next-experiment suggestions from local rules.
- Extend non-Qt core tests and Qt GUI smoke tests.

Out of scope:

- Real online LLM integration.
- API key handling, storage, or prompts that require a key.
- Network calls.
- Changes to `SimulationEngine` semantics.
- Changes to `PrivacyFactory` semantics.
- Changes to `AssignmentAlgorithmFactory` semantics.
- Changes to `BatchExperiment` semantics.
- Qt Graphs or Qt Charts changes.
- GoogleTest/Catch2 migration.
- New privacy mechanisms or assignment algorithms.

## Architecture

The `agent` module gains assistant-facing Qt-free types and implementations. The `gui` module gains a new widget that adapts Qt input/output controls to those Qt-free types.

Planned Qt-free files:

```text
GeoTaskShield/agent/
  AssistantRequest.h
  AssistantResponse.h
  ExperimentIntent.h
  IExperimentAssistant.h
  RuleBasedAssistant.h/.cpp
  MockLLMAssistant.h/.cpp
```

Planned Qt GUI files:

```text
GeoTaskShield/gui/
  AgentAssistantWidget.h/.cpp
```

Planned modifications:

```text
GeoTaskShield/CMakeLists.txt
GeoTaskShield/gui/MainWindow.h/.cpp
GeoTaskShield/gui/BatchResultsWidget.h/.cpp
GeoTaskShield/tests/test_core.cpp
GeoTaskShield/gui/tests/test_gui_smoke.cpp
```

The assistant should reuse `experiment/BatchResultRecord.h` and may reuse `BatchResultModel` summary methods internally. It should not duplicate CSV parsing logic or depend on Qt.

## Data Types

`ExperimentIntent` represents parsed user intent:

- `originalText`
- optional `workerCount`
- optional `taskCount`
- `privacyTypes`
- `algorithmTypes`
- `metricNames`
- `compareRequested`
- `compareTarget`
- `warnings`

`privacyTypes` should use existing factory-facing identifiers where possible:

- `grid`
- `k-anonymity`
- `laplace`

`algorithmTypes` should use existing factory-facing identifiers where possible:

- `nearest`
- `score`
- `hungarian`

`metricNames` should use existing internal metric names where possible:

- `completionRate`
- `averageTrueDistance`
- `totalReward`
- `averagePrivacyLoss`
- `runtimeMs`
- `userLoadStdDev`
- `fairnessIndex`
- `privacyUtilityRatio`
- `timeoutRate`

`AssistantRequest` contains:

- `prompt`
- `batchResults`
- `sourceLabel`

`batchResults` is a `std::vector<BatchResultRecord>` representing the rows the GUI wants analyzed. For the GUI workflow, this should be the current filtered rows from `Batch Results`.

`AssistantResponse` contains:

- `intent`
- `intentPreviewMarkdown`
- `analysisMarkdown`
- `warnings`
- `success`

`IExperimentAssistant` exposes one Qt-free operation:

```text
AssistantResponse analyze(const AssistantRequest& request) const
```

The exact C++ signature can use references and `[[nodiscard]]`, but it must not use Qt types.

## Rule-Based Assistant

`RuleBasedAssistant` is the default production assistant for Phase 11.

Intent parsing should be deterministic and local. It should recognize English and existing Phase 4 Chinese-friendly patterns where practical, including:

- Worker/task counts such as `100 workers`, `100 users`, `100 worker`, `50 tasks`.
- Privacy terms such as `grid`, `k-anonymity`, `k anonymity`, `k=5`, `laplace`.
- Algorithm terms such as `nearest`, `nearest greedy`, `score`, `score greedy`, `hungarian`.
- Metric terms such as `completion`, `completion rate`, `privacy utility`, `privacy loss`, `fairness`, `runtime`, `timeout`, `reward`, `distance`.
- Compare terms such as `compare`, `versus`, `vs`, privacy comparison, algorithm comparison, and metric comparison.

The parser does not need perfect natural-language understanding. Unknown terms should not fail the request; they should appear as warnings or be ignored if unrelated.

Analysis generation should:

1. Build an intent preview from parsed fields.
2. Analyze the provided batch result rows.
3. Identify:
   - max `completionRate`
   - max `privacyUtilityRatio`
   - min `averagePrivacyLoss`
   - max `fairnessIndex`
4. Include source identity for each best row:
   - `scenario`
   - `privacy`
   - `algorithm`
5. Add short interpretation text using local rules.
6. Add next-experiment suggestions.

Suggested local rules:

- If completion winner and privacy-loss winner are different rows, suggest a parameter sweep around the winning privacy mechanisms to inspect the utility/privacy tradeoff.
- If fairness is materially lower than the best completion row, suggest comparing assignment algorithms under the same privacy mechanism.
- If the prompt asks for comparison but the current filtered rows include only one privacy or algorithm value, suggest clearing filters or loading a broader CSV.
- If no rows are available, suggest loading `phase5_batch_results.csv` in `Batch Results` before rerunning analysis.

## Mock LLM Assistant

`MockLLMAssistant` implements `IExperimentAssistant` but remains fully local and deterministic.

It exists to prove that the assistant interface can support a future LLM-backed implementation without changing the GUI or tests. It may return a clearly labeled simulated response based on the same `AssistantRequest`, but it must:

- Not use network APIs.
- Not read API keys.
- Not read environment variables.
- Not add dependency on any online service.
- Not be selected by default in the GUI unless a test explicitly constructs it.

## Markdown Output

`analysisMarkdown` should be suitable for direct preview through `QTextEdit::setMarkdown` and for `.md` export.

Required sections:

```text
# GeoTaskShield Agent Assistant Analysis

## Parsed Intent
...

## Data Scope
...

## Key Conclusions
...

## Next Experiment Suggestions
...
```

`Key Conclusions` must include all four required result summaries when rows exist:

- Best completion rate.
- Best privacy-utility ratio.
- Lowest average privacy loss.
- Best fairness index.

Each conclusion should include the numeric value and the source record identity. Percent metrics such as completion and timeout can be rendered as percentages; raw metrics should keep concise fixed precision.

For empty rows, `Key Conclusions` should explicitly say that no batch rows are available for metric analysis.

## GUI Design

`MainWindow` should add a third tab:

```text
Simulation | Batch Results | Agent Assistant
```

`AgentAssistantWidget` layout:

- Top input area:
  - Multi-line natural-language input box.
  - `Analyze` button.
- Middle preview area:
  - Parsed intent preview.
  - Markdown analysis preview.
- Bottom or side actions:
  - `Export Markdown` button, optional but preferred.

The widget should not own CSV loading. It should use the current filtered records exposed by `BatchResultsWidget`.

The minimum acceptable interaction:

1. User loads `phase5_batch_results.csv` in `Batch Results`.
2. User optionally applies privacy/algorithm filters.
3. User opens `Agent Assistant`.
4. User enters a prompt such as:

```text
Compare privacy mechanisms for 50 tasks and explain the best completion and privacy utility tradeoff.
```

5. User clicks `Analyze`.
6. GUI shows parsed intent and Markdown analysis based on the current filtered rows.

If the user skips loading CSV, the assistant still shows parsed intent and a Markdown explanation that there are no batch rows to analyze.

## GUI/Core Boundary

Qt types may appear in:

```text
GeoTaskShield/gui/
```

Qt types must not appear in:

```text
GeoTaskShield/agent/
GeoTaskShield/experiment/
GeoTaskShield/model/
GeoTaskShield/simulation/
GeoTaskShield/privacy/
GeoTaskShield/assignment/
GeoTaskShield/evaluation/
GeoTaskShield/data/
```

The GUI converts between `QString` and `std::string`, and between widget state and standard C++ records. The assistant only sees `AssistantRequest`.

## Batch Results Integration

`BatchResultsWidget` should expose a small non-mutating method for the assistant workflow, for example:

```text
std::vector<BatchResultRecord> currentFilteredRecords() const
```

This method should return the same rows used by the table, chart, CSV export, and Markdown export under current filters. It should not reinterpret the table sorting order unless implementation naturally reuses the visible record vector.

`MainWindow` can wire `AgentAssistantWidget` to a provider callback or direct pointer to `BatchResultsWidget`. The assistant widget should not reach into table widgets or parse UI text to reconstruct records.

## Testing

Extend lightweight non-Qt core tests in `GeoTaskShield/tests/test_core.cpp`:

- `RuleBasedAssistant` parses worker and task counts.
- `RuleBasedAssistant` parses privacy mechanisms.
- `RuleBasedAssistant` parses assignment algorithms.
- `RuleBasedAssistant` parses metric terms.
- `RuleBasedAssistant` detects compare intent.
- `RuleBasedAssistant` generates an intent preview.
- `RuleBasedAssistant` analyzes sample `BatchResultRecord` rows and includes:
  - best `completionRate`
  - best `privacyUtilityRatio`
  - lowest `averagePrivacyLoss`
  - best `fairnessIndex`
- Empty input rows produce a useful Markdown message instead of failing.
- `MockLLMAssistant` returns deterministic local output with no network or API-key dependency.

Extend GUI smoke tests in `GeoTaskShield/gui/tests/test_gui_smoke.cpp`:

- `MainWindow` exposes an `Agent Assistant` tab/widget.
- The assistant widget has input, analyze, intent preview, Markdown preview, and export action if implemented.
- Loading a CSV in `Batch Results`, then analyzing a sample prompt in `Agent Assistant`, produces non-empty intent and Markdown text.
- Markdown output contains the required analysis title and all four required metric conclusion labels.

Verification commands:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure'
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

## Acceptance Criteria

- Non-Qt core tests pass.
- Qt GUI smoke tests pass.
- `Agent Assistant` tab exists.
- The tab provides a natural-language input box.
- The tab provides an `Analyze` button.
- The tab previews parsed experimental intent.
- The tab previews Markdown analysis.
- Markdown export exists if included in implementation.
- A sample prompt produces structured intent and Markdown analysis.
- Analysis uses current Batch Results filtered records when available.
- Markdown conclusions include best `completionRate`, best `privacyUtilityRatio`, lowest `averagePrivacyLoss`, and best `fairnessIndex`.
- The assistant provides next-experiment suggestions.
- No API key is added to source, docs, tests, generated reports, or commits.
- No network dependency is introduced.
- `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, and `BatchExperiment` semantics remain unchanged.
- Qt types remain limited to the `gui` module.
- No Qt Graphs or Qt Charts work is introduced.
- GoogleTest migration is not performed.
- No new algorithm is added.
- Work happens on `feature/phase11-intelligent-assistant` from `develop`.

## Non-Goals for Future Clarity

Phase 11 does not decide how a real online LLM will be integrated. A later phase can add a real implementation of `IExperimentAssistant` that reads keys only from runtime environment variables and is disabled by default, but that future work must have its own explicit spec and security review.
