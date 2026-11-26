#!/bin/sh
# Fetches the exact SDK headers the build needs into third_party/ (only the needed folders, pinned commits).
set -e; cd "$(dirname "$0")"
MMS=4e031eb571f5978eea8207b5d915c9cd0e8d6c58   # alliedmodders/metamod-source
SDK=47f0ea659bc70718f9c41a7830b6ad943a68ba09   # alliedmodders/hl2sdk (css branch)
get() { # name repo sha folder
	[ "$(git -C third_party/$1 rev-parse HEAD 2>/dev/null)" = "$3" ] && return
	rm -rf third_party/$1; mkdir -p third_party/$1; cd third_party/$1
	git init -q; git remote add origin https://github.com/alliedmodders/$2
	git sparse-checkout set $4
	git fetch -q --depth 1 --filter=blob:none origin $3; git checkout -q FETCH_HEAD; cd ../..
}
get metamod-source metamod-source $MMS core
get hl2sdk-css hl2sdk $SDK public
