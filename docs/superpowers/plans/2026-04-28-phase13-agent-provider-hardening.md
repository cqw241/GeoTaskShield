# Phase 13 Agent Provider Hardening Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Harden the optional OpenAI-compatible assistant provider with timeout metadata, robust fallback behavior, and non-blocking GUI execution.

**Architecture:** Keep `IExperimentAssistant` as the boundary. Add timeout metadata to `HttpRequest`, make `OpenAICompatibleAssistant` pass configured timeout values to the transport, and move only the GUI provider call off the UI thread. Core modules remain Qt-free.

**Tech Stack:** C++20, CMake, WinHTTP, Qt Widgets, lightweight CTest executables.

---

### Task 1: Core Provider Timeout And Fallback Tests

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`

- [ ] **Step 1: Add failing tests**

Add tests near existing `OpenAICompatibleAssistant` fake HTTP tests:

```cpp
llmConfig.requestTimeoutMs = 4321;
fakeHttp->response.success = true;
fakeHttp->response.statusCode = 200;
fakeHttp->response.body =
    R"({"choices":[{"message":{"role":"assistant","content":"# LLM Markdown"}}]})";
const AssistantResponse timedResponse = llmAssistant.analyze(llmRequest);
require(timedResponse.success, "Provider should still succeed with timeout configured.");
require(fakeHttp->lastRequest.timeoutMs == 4321,
        "OpenAICompatibleAssistant should pass configured timeout to HTTP requests.");

fakeHttp->response = HttpResponse{false, 504, {}, "request timed out"};
const AssistantResponse timeoutResponse = llmAssistant.analyze(llmRequest);
require(!timeoutResponse.success,
        "OpenAICompatibleAssistant should report HTTP transport failures.");
require(contains(timeoutResponse.analysisMarkdown, "request timed out"),
        "Provider fallback should include the transport failure message.");

fakeHttp->response = HttpResponse{true, 200, R"({"choices":[]})", {}};
const AssistantResponse emptyContentResponse = llmAssistant.analyze(llmRequest);
require(!emptyContentResponse.success,
        "OpenAICompatibleAssistant should reject provider responses without Markdown content.");
require(contains(emptyContentResponse.analysisMarkdown, "Local Analysis Fallback"),
        "Provider fallback should preserve local analysis for empty provider content.");
```

- [ ] **Step 2: Run red test**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug --target GeoTaskShieldTests'
```

Expected: compile fails because `LLMProviderConfig::requestTimeoutMs` and `HttpRequest::timeoutMs` do not exist.

### Task 2: Core Provider Timeout Implementation

**Files:**
- Modify: `GeoTaskShield/agent/HttpClient.h`
- Modify: `GeoTaskShield/agent/OpenAICompatibleAssistant.h`
- Modify: `GeoTaskShield/agent/OpenAICompatibleAssistant.cpp`
- Modify: `GeoTaskShield/agent/WinHttpClient.cpp`

- [ ] **Step 1: Add timeout fields**

Add `int timeoutMs{15000};` to `HttpRequest` and `int requestTimeoutMs{15000};` to `LLMProviderConfig`.

- [ ] **Step 2: Pass timeout through provider**

Set `httpRequest.timeoutMs = config_.requestTimeoutMs;` before calling `postJson()`.

- [ ] **Step 3: Apply WinHTTP timeouts**

Call `WinHttpSetTimeouts(httpRequest.value, request.timeoutMs, request.timeoutMs, request.timeoutMs, request.timeoutMs)` after `WinHttpOpenRequest`.

- [ ] **Step 4: Run green core test**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug --target GeoTaskShieldTests && ctest --test-dir out\build\x64-debug --output-on-failure -R GeoTaskShieldCoreTests'
```

Expected: `GeoTaskShieldCoreTests` passes.

### Task 3: GUI Async State Tests

**Files:**
- Modify: `GeoTaskShield/gui/tests/test_gui_smoke.cpp`
- Modify: `GeoTaskShield/gui/AgentAssistantWidget.h`

- [ ] **Step 1: Add failing GUI smoke expectations**

Add smoke checks:

```cpp
require(assistantWidget->hasProviderStatusForTesting(),
        "AgentAssistantWidget should expose provider status text.");
assistantWidget->setProviderForTesting("Local rule-based");
assistantWidget->analyzeForTesting();
require(assistantWidget->providerStatusForTesting().contains("Local", Qt::CaseInsensitive),
        "Local assistant analysis should leave visible provider status.");
require(!assistantWidget->isAnalyzingForTesting(),
        "Local assistant analysis should finish synchronously.");
```

- [ ] **Step 2: Run red GUI test**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug-qt --target GeoTaskShieldGuiSmokeTests'
```

Expected: compile fails because the testing helpers do not exist.

### Task 4: GUI Async Provider Implementation

**Files:**
- Modify: `GeoTaskShield/gui/AgentAssistantWidget.h`
- Modify: `GeoTaskShield/gui/AgentAssistantWidget.cpp`

- [ ] **Step 1: Add status helpers and state**

Add `QLabel* providerStatusLabel_{}`, `bool analyzing_{}`, and testing helpers `hasProviderStatusForTesting()`, `providerStatusForTesting()`, and `isAnalyzingForTesting()`.

- [ ] **Step 2: Add non-blocking DashScope flow**

Use `QThread::create` for the DashScope path. Disable Analyze while running, set status to analyzing, then update previews and status when the thread finishes.

- [ ] **Step 3: Keep local path simple**

The local rule-based path remains synchronous and sets status to local analysis complete.

- [ ] **Step 4: Run green GUI test**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt --target GeoTaskShieldGuiSmokeTests && ctest --test-dir out\build\x64-debug-qt --output-on-failure -R GeoTaskShieldGuiSmokeTests'
```

Expected: `GeoTaskShieldGuiSmokeTests` passes.

### Task 5: Documentation And Full Verification

**Files:**
- Modify: `README.md`
- Modify: `HANDOFF.md`
- Modify: `CHANGELOG.md`

- [ ] **Step 1: Update docs**

Document Phase 13 provider hardening, timeout/fallback behavior, and GUI non-blocking provider call.

- [ ] **Step 2: Run full verification**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure'
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

Expected: non-Qt `1/1` and Qt `2/2` tests pass.

- [ ] **Step 3: Commit**

Stage only Phase 13 source/docs files, excluding `task_plan.md`, `findings.md`, and `progress.md`.

Commit message:

```bash
feat(agent): harden llm provider experience
```

