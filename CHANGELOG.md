# Changelog

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
