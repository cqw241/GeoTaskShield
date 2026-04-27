# Phase 4 AIAgent and Report Generation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a local natural-language experiment agent and Markdown report generator.

**Architecture:** Add a focused `agent` module to `GeoTaskShieldCore`. Keep parsing, orchestration, and report formatting separate. Do not introduce network dependencies or commit secrets.

**Tech Stack:** C++20, existing custom core tests, CMake, existing strategy factories and `SimulationEngine`.

---

### Task 1: Parser Test and Types

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`
- Create: `GeoTaskShield/agent/ExperimentRequest.h`
- Create: `GeoTaskShield/agent/RuleBasedConfigParser.h`
- Create: `GeoTaskShield/agent/RuleBasedConfigParser.cpp`

- [ ] Write failing tests for Chinese and English prompts.
- [ ] Verify the build fails because parser headers do not exist.
- [ ] Implement `ExperimentRequest` and parser with conservative regex/string matching.
- [ ] Verify parser tests pass.

### Task 2: Report Generator

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`
- Create: `GeoTaskShield/agent/ExperimentReport.h`
- Create: `GeoTaskShield/agent/ReportGenerator.h`
- Create: `GeoTaskShield/agent/ReportGenerator.cpp`

- [ ] Write a failing test that expects Markdown title, strategy names, metrics table, and summary.
- [ ] Implement report data structures and Markdown generation.
- [ ] Verify report generator tests pass.

### Task 3: Experiment Agent

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`
- Create: `GeoTaskShield/agent/ExperimentAgent.h`
- Create: `GeoTaskShield/agent/ExperimentAgent.cpp`

- [ ] Write a failing test that runs a natural-language request through the agent.
- [ ] Implement orchestration through existing factories and `SimulationEngine`.
- [ ] Support single-run and `compare privacy` requests.
- [ ] Verify agent tests pass.

### Task 4: Build and Console Demo

**Files:**
- Modify: `GeoTaskShield/CMakeLists.txt`
- Create: `GeoTaskShield/app/agent_demo/main.cpp`

- [ ] Add agent sources to `GeoTaskShieldCore`.
- [ ] Add `GeoTaskShieldAgentDemo` executable.
- [ ] Run full non-Qt build and CTest.
- [ ] Run demo executable and inspect Markdown output.
