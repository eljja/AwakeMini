#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
mkdir -p dist build
for arch in i686 x86_64; do
  prefix="$arch-w64-mingw32"
  "$prefix-windres" -I src src/awake-mini.rc "build/$arch-res.o"
  # Match PE file alignment so resource reads include their padding.
  "$prefix-objcopy" --set-section-alignment .rsrc=512 "build/$arch-res.o"
  output=AwakeMini.exe
  if [ "$arch" = x86_64 ]; then output=AwakeMini-x64.exe; fi
  for language in auto ko en; do
  force=0
  if [ "$language" = ko ]; then force=1; fi
  if [ "$language" = en ]; then force=2; fi
  base="${output%.exe}"
  target="$base-$language.exe"
  "$prefix-gcc" -DAM_FORCE_LANGUAGE="$force" -std=c11 -Os -s -Wall -Wextra -Werror -fno-ident \
    -ffunction-sections -fdata-sections -static-libgcc -mwindows \
    src/awake-mini.c src/update-pause.c src/startup.c src/language.c src/blackout.c src/win-b-hook.c src/state-test.c "build/$arch-res.o" -o "dist/$target" \
    -Wl,--gc-sections,--no-insert-timestamp,--dynamicbase,--nxcompat \
    -Wl,--major-subsystem-version,6,--minor-subsystem-version,1 \
    -lgdi32 -luser32 -lshell32 -ladvapi32 -lwtsapi32 -lktmw32 -lruntimeobject -lole32 -luuid
  done
done
