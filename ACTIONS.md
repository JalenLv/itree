# GitHub Actions

This repository includes two GitHub Actions workflows for build and release automation.

## Build artifacts in CI

`.github/workflows/build.yml` builds `itree` on Linux and macOS, packages the binary with the project docs, and uploads the resulting archives as workflow artifacts only.

It runs automatically on pushes to `main` and on pull requests targeting `main`. You can also run it manually from the **Actions** tab with `workflow_dispatch`.

1. Open the **Actions** tab.
2. Select **Build Artifacts**.
3. Click **Run workflow** and choose the ref if you want a manual run.

The workflow accepts an optional `version` input used only for artifact filenames.

## Release workflow

`.github/workflows/release.yml` supports both manual runs and tag-based releases.

- `workflow_dispatch` is available in the GitHub UI because the workflow lives on the default branch
- `push.tags: v*` is the production trigger for real releases.

For safe testing, run the release workflow manually and leave `draft_release` enabled. That creates a draft release so you can verify the uploaded assets before publishing anything publicly.

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

1. Use `build.yml` to verify Linux and macOS artifacts in CI or from a manual run.
2. Use `release.yml` manually with `draft_release` enabled when you want to inspect a draft release.
3. Publish the draft if everything looks right, or rerun the workflow after fixes.
4. For a normal release, create and push a tag such as `v0.1.0`.
