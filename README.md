# Doom II RPG — RetroFW Port (RS-97) + Russian Localization

Port of **Doom II RPG** (based on `DoomIIRPG-RE`) for **RS-97** handheld consoles running **RetroFW**, featuring TinyGL performance optimizations and optional full Russian localization.

## Features
- **TinyGL Performance Boost:** Rebuilt `libtinygl` with `-O3 -fomit-frame-pointer`, Fast Inverse Square Root (`TGL_FEATURE_FISR`), and disabled unused stipple/multithreading overhead for smoother 3D rendering on MIPS32.
- **Full Russian Localization (Optional):** All 1,931 strings translated (menus, NPC/computer dialogs, PDA logs, item descriptions, and floating speech bubbles) with custom Cyrillic bitmap fonts matched to the original palette.
- **Native Hardware Support:** `320×240` (`RGB565`) native output, 30 FPS limiter, working `22050 Hz` audio, and self-contained saves.

## Controls (RS-97)

| Button | Action |
| :--- | :--- |
| **D-Pad** | Move / Turn / Menu Navigation |
| **A** | Confirm / Attack / Interact |
| **B** | Back / Cancel |
| **X / Y** | Next / Previous Weapon |
| **L / R** | Strafe Left / Right |
| **Start** | In-game Menu / Pause |
| **Select + R** | Toggle Fullscreen / Aspect Ratio |
| **Select + Start** | Exit to RetroFW |

## Building from Source

Set `TOOLCHAIN` to your RetroFW Buildroot toolchain file (e.g., `~/retrofw-buildroot/output/host/usr/share/buildroot/toolchainfile.cmake`):

1. **Build TinyGL for RetroFW:**
   ```bash
   cd third_party_libs/tinygl
   mkdir -p build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN -DCMAKE_C_FLAGS="-O3 -fomit-frame-pointer -fno-math-errno -fno-trapping-math"
   make -j$(nproc)
   cd ../../..
   ```

2. **Build Doom II RPG:**
   ```bash
   mkdir -p build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN
   make -j$(nproc)
   cd ..
   ```

3. **Package `.opk` (English or Russian):**
   Place your legally obtained `Doom 2 RPG.ipa` into the repository root, then run:
   - **English version (original assets):**
     ```bash
     ./build_opk.sh en
     ```
   - **Russian version (with full localization & Cyrillic fonts):**
     ```bash
     ./build_opk.sh ru
     ```

4. Copy `DoomIIRPG_EN.opk` or `DoomIIRPG_RU.opk` to `apps/` on your SD card.

## Credits
- Original reverse engineering: **Erick194** (`DoomIIRPG-RE`)
- RetroFW port, TinyGL optimizations & Russian localization
