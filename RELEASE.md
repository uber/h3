# Release Process

1. Create a PR "Preparing for release X.Y.Z" against master branch
    * Alter CHANGELOG.md from `[Unreleased]` to `[X.Y.Z] YYYY-MM-DD`
    * Run `make update-version` and give `X.Y.Z` when prompted
    * Check that all merges that need to be in the changelog are present

2. Create a release "Release X.Y.Z" on Github
    * Create Tag `vX.Y.Z`
    * Copy CHANGELOG.md into the release notes

3. Publish the website to display the latest docs
    * `make publish-website`
