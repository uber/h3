# Release Process

1. Create a PR "Preparing for release X.Y.Z" against master branch
    * Update VERSION to `X.Y.Z`
    * Alter CHANGELOG.md from `[Unreleased]` to `[X.Y.Z] YYYY-MM-DD`

2. Create a release "Release X.Y.Z" on Github
    * Create Tag `vX.Y.Z`
    * Copy CHANGELOG.md into the release notes

3. Publish the website to display the latest docs
    * `make publish-website`
