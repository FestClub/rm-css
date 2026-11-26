#!/bin/sh
# Builds dist/ (drop dist/addons into the server cstrike/). Run ./deps.sh once to fetch the SDK headers.
set -e; cd "$(dirname "$0")"
M=third_party/metamod-source; H=third_party/hl2sdk-css; D=dist/addons
mkdir -p $D/rm_css/bin $D/metamod
g++ -std=c++17 src/test.cpp -o /tmp/rm_test && /tmp/rm_test
clang++ -m32 -shared -fPIC -O2 -std=c++17 -fno-exceptions -w -msse2 -mfpmath=sse -DSOURCE_ENGINE=6 -DSE_CSS=6 -DSE_ORANGEBOX=3 -DGAME_DLL -DCOMPILER_GCC -D_LINUX -DLINUX -DPOSIX -DGNUC -DNO_HOOK_MALLOC -DNO_MALLOC_OVERRIDE -DCSTRIKE_DLL -DHAVE_STDINT_H -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -I$M/core -I$M/core/sourcehook -I$H/public -I$H/public/engine -I$H/public/mathlib -I$H/public/vstdlib -I$H/public/tier0 -I$H/public/tier1 -I$H/public/game/server src/rm_css.cpp -static-libstdc++ -static-libgcc -lpthread -s -o $D/rm_css/bin/rm_css.so
printf '"Metamod Plugin"\n{\n\t"alias"\t"rm"\n\t"file"\t"addons/rm_css/bin/rm_css"\n}\n' > $D/metamod/rm_css.vdf
cp config/rm.cfg $D/rm_css/rm.cfg
