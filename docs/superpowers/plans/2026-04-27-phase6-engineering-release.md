# Phase 6 Engineering Release Plan

## Success Criteria

- Phase 5 is merged into `develop`.
- Release work happens on `release/phase6-engineering-release`.
- Unused `GeoTaskShield.cpp` and `GeoTaskShield.h` are removed.
- `.clang-format`, warning options, release Qt preset, and packaging script exist.
- README and HANDOFF describe Phase 6 release usage.
- Debug, Qt, Release, and package verification pass.
- Release is committed, merged to `main` and `develop`, tagged `v0.6.0`, and pushed.

## Steps

1. Integrate Phase 5.
   - Verify: `git log --oneline --graph` shows Phase 5 merge on `develop`.
2. Apply engineering cleanup.
   - Remove unused Visual Studio template files.
   - Add `.clang-format`.
   - Add warning options and `x64-release-qt` preset.
   - Verify: CMake configure still succeeds.
3. Add package helper and release docs.
   - Add `scripts/package_windows.ps1`.
   - Update README, HANDOFF, planning files, and changelog.
   - Verify: package script creates the expected ZIP under `out/package`.
4. Run release verification.
   - Verify non-Qt Debug, Qt Debug, Release Qt, package script, secret scan, and diff checks.
5. Publish through Git Flow.
   - Commit release branch.
   - Merge release branch into `main`.
   - Tag `v0.6.0`.
   - Merge release branch back into `develop`.
   - Push branches and tag.

## Risk Controls

- Do not run broad formatting across existing files in Phase 6.
- Do not add `/WX`; warnings should surface issues without blocking local toolchain differences.
- Keep package output ignored under `out/`.
- Do not commit API keys, Qt DLLs, build outputs, or ZIP artifacts.
