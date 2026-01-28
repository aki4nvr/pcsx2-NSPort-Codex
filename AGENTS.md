# AGENTS

## Project summary
- PCSX2 is a PlayStation 2 emulator written in C++ (C++20) and built with CMake.
- Core emulator code lives under `pcsx2/` with shared utilities in `common/`.
- The Qt UI is in `pcsx2-qt/`, with additional tools in `pcsx2-gsrunner/` and `tools/`.

## Repo layout
- `common/`: shared libraries, platform helpers, build props.
- `pcsx2/`: emulator core, CPU/GS/SPU2/VU/etc.
- `pcsx2-qt/`: Qt-based UI and host integration.
- `pcsx2-gsrunner/`: standalone GS runner tool.
- `tests/`: test harness (GoogleTest via CMake when enabled).
- `cmake/`: CMake modules and build options.
- `bin/`: runtime resources, docs, utilities.
- `3rdparty/`: vendored dependencies (avoid editing unless requested).

## Build
- Out-of-tree builds are required (top-level `CMakeLists.txt` errors on in-tree).
- CMake version range: 3.16 to 3.25.
- Example (single-config generator):
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Devel`
  - `cmake --build build`
- Common options in `cmake/BuildParameters.cmake`:
  - `ENABLE_QT_UI` (default ON)
  - `ENABLE_TESTS` (default ON)
  - `ENABLE_GSRUNNER` (default OFF)
- On Linux (UNIX, not Apple), binaries are emitted to `build/bin`.
- Apple Silicon builds are warned as incomplete in CMake; prefer `-DCMAKE_OSX_ARCHITECTURES=x86_64` unless you are working on ARM64 recompilers.

## Tests
- Tests are enabled by `-DENABLE_TESTS=ON` (default).
- Run with CTest from the build directory, e.g. `ctest` or `ctest --test-dir build`.

## Docs
- Docs sources: `pcsx2/Docs/`. Release artifacts: `bin/Docs`.
- `pcsx2/Docs/gen-docs.sh` generates PDFs (requires `pandoc` and LaTeX). Use `OUT_DIR=<path>` to override output.

## Conventions
- Formatting is driven by `.clang-format` (4-space indents, custom brace rules).
- Avoid editing files in `3rdparty/` unless explicitly asked.
