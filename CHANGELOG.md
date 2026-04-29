# Changelog

## v0.10.0 - 2026-04-28

### Added

- Phase 13 provider hardening for the optional OpenAI-compatible assistant provider.
- `DASHSCOPE_TIMEOUT_MS` runtime configuration for provider request timeout.
- Provider status text in the `Agent Assistant` GUI tab.

### Changed

- DashScope provider analysis now runs off the GUI thread to avoid blocking the Qt UI.
- Provider failures, timeouts, empty responses, and malformed responses preserve local rule-based fallback analysis.

### Verification

- Non-Qt core tests cover timeout propagation, invalid timeout fallback, transport failure fallback, empty content fallback, and malformed response fallback.
- GUI smoke tests cover provider status and missing-key fallback behavior without using a real API key.

## v0.9.0 - 2026-04-28

### Added

- Phase 12 optional OpenAI-compatible LLM provider for `IExperimentAssistant`.
- Aliyun Bailian / DashScope provider selection in the `Agent Assistant` GUI tab.
- Environment-variable configuration for `DASHSCOPE_API_KEY`, `DASHSCOPE_MODEL`, and `DASHSCOPE_BASE_URL`.
- WinHTTP-based Qt-free HTTP transport with fake-transport core tests.
- Demo guide and checklist: `docs/demo/v0.9.0-gui-demo-guide.md`.

### Changed

- `Agent Assistant` remains local rule-based by default; real provider calls only run when explicitly selected.
- Updated README and handoff documentation for Demo Readiness.
- Updated Windows packaging defaults to produce `GeoTaskShield-v0.9.0-windows-x64.zip`.

### Verification

- Non-Qt Debug build and CTest.
- Qt Debug build and CTest.
- Qt Release build.
- Windows packaging script.

## v0.8.0 - 2026-04-28

### Added

- Phase 9 GUI Markdown preview and export entries for `Batch Results`.
- Phase 9 GUI filtered CSV export entry for `Batch Results`.
- Qt-free Markdown report generation for currently filtered batch result rows.
- Qt-free CSV generation for currently filtered batch result rows.
- Demo guide and screenshot checklist: `docs/demo/v0.8.0-gui-demo-guide.md`.

### Changed

- Updated README and handoff documentation for the Phase 9 GUI export workflow.
- Updated Windows packaging defaults to produce `GeoTaskShield-v0.8.0-windows-x64.zip`.

### Verification

- Non-Qt Debug build and CTest.
- Qt Debug build and CTest.
- Qt Release build.
- Windows packaging script.

## v0.7.0 - 2026-04-28

### Added

- Qt GUI `Batch Results` tab for loading and analyzing Phase 5-style batch CSV files.
- Batch result CSV parsing and analysis helpers in the Qt-free `experiment` module.
- Lightweight self-drawn metric bar chart for batch result comparison.
- Demo guide and screenshot checklist: `docs/demo/v0.7.0-gui-demo-guide.md`.

### Changed

- Updated README and handoff documentation for the Phase 7 GUI visualization workflow.
- Updated Windows packaging defaults to produce `GeoTaskShield-v0.7.0-windows-x64.zip`.
- Included demo guide files in the Windows release package.

### Verification

- Non-Qt Debug build and CTest.
- Qt Debug build and CTest.
- Qt Release build.
- Windows packaging script.

## v0.6.0 - 2026-04-27

### Added

- Windows packaging helper: `scripts/package_windows.ps1`.
- Qt Release preset: `x64-release-qt`.
- Project formatting convention: `.clang-format`.
- Project version metadata in CMake.

### Changed

- Added compiler warning options for project targets.
- Updated README and handoff documentation for release usage.
- Integrated Phase 5 batch experiment work into the release line.

### Removed

- Removed unused Visual Studio template files `GeoTaskShield/GeoTaskShield.cpp` and `GeoTaskShield/GeoTaskShield.h`.

### Verification

- Non-Qt Debug build and CTest.
- Qt Debug build and CTest.
- Qt Release build.
- Windows packaging script.
