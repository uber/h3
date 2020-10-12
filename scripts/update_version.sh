#!/usr/bin/env bash
# Copyright 2019 Uber Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script is used interactively as part of the H3 release process
# (RELEASE.md) to update the version number in the VERSION file. Before
# writing the new version to the file, changelog information is presented
# for verification. It is invoked by the make target `update-version`.

set -eo pipefail

CURRENT_VERSION=$(<VERSION)
echo "Current version: $CURRENT_VERSION"
CURRENT_TAG="v$CURRENT_VERSION"

if ! git rev-parse $CURRENT_TAG -- > /dev/null 2>&1; then
    echo "Could not locate $CURRENT_TAG as a Git revision."
    exit 1
fi

REVISION_RANGE="$CURRENT_TAG..HEAD"

read -p "Next version: " NEXT_VERSION

echo -e "\n * Changelog entries *"
git diff $REVISION_RANGE -- CHANGELOG.md
echo -e "\n * Committed merges *"
git log --merges --oneline $REVISION_RANGE

echo
read -p "Are all changes in the changelog [y/N]? " CHANGELOG_OK
if [ "y" = "$CHANGELOG_OK" ] || [ "Y" = "$CHANGELOG_OK" ]; then
    echo $NEXT_VERSION > VERSION
    echo "Wrote $NEXT_VERSION to the VERSION file"
else
    echo "Cancelled - did not write VERSION file"
    echo "Please update the CHANGELOG with the appropriate entries before bumping the version."
    exit 2
fi
