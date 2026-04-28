# Phase 6 Engineering Release Design

## Goal

Turn the current Phase 1-5 implementation into a cleaner C++/Qt deliverable without changing algorithm behavior.

## Scope

Phase 6 is an engineering and release-preparation phase. It should:

- integrate Phase 5 into `develop`;
- remove unused Visual Studio template entry files;
- add project-level formatting and warning conventions;
- add a Windows packaging helper for console, agent, batch, and Qt GUI executables;
- improve README/HANDOFF release documentation;
- verify debug, Qt, release, and packaging paths;
- publish the release through Git Flow branches and an annotated tag.

## Non-Goals

- No new privacy mechanism or assignment algorithm.
- No GUI redesign.
- No online LLM integration.
- No committed secrets or generated build artifacts.

## Release Shape

Use `release/phase6-engineering-release` as the release preparation branch. After verification:

- merge the release branch into `main`;
- create tag `v0.6.0`;
- merge the release branch back into `develop`;
- push `main`, `develop`, release branch, and tag.

## Packaging

Add `scripts/package_windows.ps1`. The script builds the `x64-release-qt` preset, stages the executable output directory, copies README/HANDOFF/sample reports, and creates:

```text
out/package/GeoTaskShield-v0.6.0-windows-x64.zip
```

The ZIP is a generated artifact and must stay untracked.

## Verification

- Non-Qt Debug build and CTest.
- Qt Debug build and CTest.
- Release Qt build.
- Windows packaging script.
- Secret scan.
- `git diff --check`.
