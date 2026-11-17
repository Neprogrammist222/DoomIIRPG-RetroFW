#!/bin/bash
set -e

LANG_MODE=${1:-en}
IPA_FILE="Doom 2 RPG.ipa"
BIN_FILE="build/DoomIIRPG"

if [ ! -f "$IPA_FILE" ]; then
    echo "Error: '$IPA_FILE' not found in project root!"
    echo "Please place your legally obtained 'Doom 2 RPG.ipa' here first."
    exit 1
fi

if [ ! -f "$BIN_FILE" ]; then
    echo "Error: Compiled binary '$BIN_FILE' not found!"
    echo "Build the project with CMake first."
    exit 1
fi

rm -rf opk_tmp
mkdir -p opk_tmp

cp "$BIN_FILE" opk_tmp/DoomIIRPG
cp default.retrofw.desktop default.gcw0.desktop opk_tmp/

if [ "$LANG_MODE" = "ru" ]; then
    echo "Applying Russian localization..."
    python3 ru_patch/pack_ru.py "$IPA_FILE" "opk_tmp/Doom 2 RPG.ipa"
    OUT_NAME="DoomIIRPG_RU.opk"
else
    echo "Packaging original English version..."
    cp "$IPA_FILE" "opk_tmp/Doom 2 RPG.ipa"
    OUT_NAME="DoomIIRPG_EN.opk"
fi

mksquashfs opk_tmp "$OUT_NAME" -all-root -noappend -no-exports -no-xattrs
rm -rf opk_tmp
echo "Done! Created $OUT_NAME"
