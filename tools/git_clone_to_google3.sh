#!/bin/bash
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Crude script to clone the git skia repo into the current directory, which
# must be a CitC client.
#
# Usage:
#      ./tools/git_clone_to_google3.sh

prodcertstatus -q || (echo "Please run prodaccess." 1>&2; exit 1)
source gbash.sh || exit 2

DEFINE_string skia_rev "" "Git hash of Skia revision to clone, default latest."
gbash::init_google "$@"

set -e

# Checkout specified revision of Skia in a temp location.
TMP=$(gbash::make_temp_dir)
pushd "${TMP}"
git clone https://skia.googlesource.com/skia
cd skia
git fetch
if [[ -z "${FLAGS_skia_rev}" ]]; then
  # Retrieve latest revision.
  FLAGS_skia_rev="$(git show --format=%H --no-patch origin/master)"
fi
git checkout --detach "${FLAGS_skia_rev}"

# Rsync to google3 location.
popd
# Use multichange client in case there are too many files for nomultichange. http://b/7292343
g4 client --set_option multichange
# Use allwrite to simplify opening the correct files after rsync.
g4 client --set_option allwrite
# Filter directories added to CitC.
rsync -avzJ \
  --delete \
  --delete-excluded \
  --include=/bench \
  --include=/dm \
  --include=/gm \
  --include=/include \
  --include=/resources \
  --exclude=/src/animator \
  --include=/src \
  --include=/tests \
  --include=/third_party \
  --include=/tools \
  --include=/.git \
  '--exclude=/*/' \
  --include=/third_party/etc1 \
  --include=/third_party/ktx \
  --include=/third_party/libwebp \
  '--exclude=/third_party/*/' \
  "${TMP}/skia/" \
  "./"

# Open added/changed files for add/edit.
g4 reopen
# Revert files that are equivalent to the checked in version.
g4 revert -a

# Tell CitC to ignore .git and .gitignore.
find . \
  \( -name .git \
  -o -name .gitignore \
  \) \
  -execdir g4 revert -k \{\} \;

# Tell Git to ignore README.google and BUILD.
echo README.google >> .git/info/exclude
echo BUILD >> .git/info/exclude
g4 revert README.google
g4 revert BUILD

# Use google3 version of OWNERS.
find . \
  -name OWNERS \
  -exec git update-index --skip-worktree \{\} \; \
  -execdir g4 revert \{\} \;

# Tell git to ignore these files that have Windows line endings, because Piper
# will always change them to Unix line endings.
git update-index --skip-worktree make.bat
git update-index --skip-worktree make.py

# Tell git to ignore files left out of the rsync (i.e. "deleted" files).
git status --porcelain | \
  grep -e "^ D" | \
  cut -c 4- | \
  xargs git update-index --skip-worktree

