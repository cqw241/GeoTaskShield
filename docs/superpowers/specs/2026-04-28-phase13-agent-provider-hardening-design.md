# Phase 13 Agent Provider Hardening Design

## Goal

Phase 13 turns the optional OpenAI-compatible assistant provider from a demo entry point into a safer product feature. The work keeps `Local rule-based` as the default path and does not change simulation, privacy, assignment, evaluation, batch experiment, or reporting semantics.

## Scope

In scope:

- Add explicit provider timeout configuration to the Qt-free HTTP request path.
- Improve provider failure handling for HTTP errors, empty content, malformed provider responses, and missing configuration.
- Keep local intent parsing and local analysis as the fallback shown to users.
- Run real provider requests from the GUI without blocking the UI thread.
- Show visible GUI state while a provider request is running or unavailable.
- Cover new behavior with fake HTTP core tests and GUI smoke tests.

Out of scope:

- New privacy mechanisms or assignment algorithms.
- Changes to `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, or `BatchExperiment`.
- Storing API keys anywhere in the repository.
- Multi-turn chat memory, prompt template management UI, streaming responses, or Qt Graphs/Charts.

## Architecture

The existing `IExperimentAssistant` boundary remains the extension point. `OpenAICompatibleAssistant` continues to run `RuleBasedAssistant` first, then optionally calls the configured provider through `IHttpClient`. Phase 13 adds request timeout metadata to `HttpRequest` and maps `LLMProviderConfig` timeout settings into the HTTP transport.

The Windows transport stays in `WinHttpClient` and remains Qt-free. It applies WinHTTP timeout settings before sending the request and returns structured `HttpResponse` failures that `OpenAICompatibleAssistant` turns into Markdown fallback output.

`AgentAssistantWidget` owns GUI responsiveness. Local rule-based analysis can remain synchronous because it is local and fast. The DashScope provider path runs on a worker thread, disables the Analyze button while running, shows a status label, and applies the result back to the UI when finished. The widget does not expose API keys and does not retry automatically.

## Data Flow

1. GUI builds `AssistantRequest` from prompt and current Batch Results rows.
2. Local provider path calls `RuleBasedAssistant::analyze()` and updates previews immediately.
3. DashScope provider path starts a worker thread and marks the UI as analyzing.
4. Worker thread constructs `OpenAICompatibleAssistant`, which:
   - runs local fallback analysis,
   - validates `DASHSCOPE_API_KEY`,
   - builds the OpenAI-compatible request,
   - applies configured timeout metadata,
   - calls `IHttpClient::postJson()`,
   - returns remote Markdown or local fallback Markdown with warnings.
5. GUI receives the response, updates parsed intent, Markdown analysis, and status text.

## Error Handling

Provider failure is not fatal. Missing keys, HTTP transport failures, non-2xx responses, empty provider content, and malformed provider JSON all return `AssistantResponse::success == false` with warnings and a Markdown fallback section. The GUI surfaces this through status text and the Markdown preview, while preserving export behavior for the generated fallback report.

Timeouts are configured by milliseconds on the request object. If the real transport times out, it returns a failed `HttpResponse`; fake transports in tests can verify the timeout value without sleeping or network access.

## Testing

Core tests stay in `GeoTaskShield/tests/test_core.cpp` for Phase 13 to avoid a framework migration. New tests cover:

- provider request includes the configured timeout;
- provider HTTP failure returns fallback Markdown with warnings;
- provider success with empty content returns fallback Markdown;
- provider success with malformed JSON returns fallback Markdown.

GUI smoke tests stay in `GeoTaskShield/gui/tests/test_gui_smoke.cpp`. New tests cover:

- assistant widget exposes a running-status surface;
- local analysis leaves the widget idle after completion;
- DashScope provider path can be invoked without an API key and produces a visible unavailable/fallback state without blocking the smoke test.

