#!/bin/bash
# Copyright 2018 Uber Technologies, Inc.
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

# This file runs the build of the H3 documentation website and commits the
# results to the `gh-pages` branch.
#
# The script expects to be run from the `website` directory.

set -ex

TARGET_BRANCH=gh-pages
CURRENT_COMMIT=`git rev-parse HEAD`

npm install
npm run build
# The dist directory is not removed because it is gitignore'd.
git checkout "$TARGET_BRANCH"
cd ..
# Copy over the .gitignore file
git checkout $CURRENT_COMMIT -- .gitignore
git rm bundle-*.js styles-*.css
cp -R website/dist/* .
git add .
git commit
