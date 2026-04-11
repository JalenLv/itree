# GitHub Actions

This repository includes two GitHub Actions workflows for build and release automation.

## Build artifacts safely on a branch

`.github/workflows/build.yml` is the safe test workflow. It builds `itree` on Linux and macOS, packages the binary with the project docs, and uploads the resulting archives as workflow artifacts only.

You can test it from a feature branch without touching the GitHub Releases page:

1. Push your branch.
2. Open the **Actions** tab.
3. Select **Build Artifacts**.
4. Click **Run workflow** and choose your branch.

The workflow accepts an optional `version` input used only for artifact filenames.

## Test the release workflow without publishing a real release

`.github/workflows/release.yml` supports both manual runs and tag-based releases.

- pushes to `chore-github-actions-release` create draft test releases automatically
- `workflow_dispatch` becomes available in the GitHub UI after the workflow exists on the default branch
- `push.tags: v*` is the production trigger for real releases.

For safe pre-merge testing, push to `chore-github-actions-release`. That branch path forces `draft: true` and uses a non-production test tag like `draft-test-<sha>` so you can inspect the draft release without publishing a real release.

## Local packaging test

You can validate the packaging commands locally before relying on GitHub Actions:

```bash
./configure
make build
mkdir -p dist/itree
cp itree README.md LICENSE dist/itree/
tar -czf itree-test-linux-x86_64.tar.gz -C dist itree
sha256sum itree-test-linux-x86_64.tar.gz
```

## Maintainer release flow

1. Test `build.yml` from your branch until the artifacts look correct.
2. Push `chore-github-actions-release` and verify that `release.yml` creates a draft test release.
3. Inspect the draft release assets and rerun the branch workflow after fixes if needed.
4. After the workflow is merged to the default branch, use `workflow_dispatch` for manual releases if needed.
5. For a normal release, create and push a tag such as `v0.1.0`.
