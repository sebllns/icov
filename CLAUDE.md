# ICOV

Interactive Coder for Omnidirectional Videos: a 360 ERP image or video is encoded once, then a client decodes only the blocks of its viewport, each predicted from a neighbor it already decoded (LDPCA syndromes, rate depends on the side information).

## Repositories

- This superproject (`sebllns/icov`): plan, Unity demonstrators, renderer, network stubs, scripts, tools. These parts are legacy; the top-level `CMakeLists.txt` does not configure (it includes the missing `.cmake/AddGitSubmodule.cmake`).
- `core/` submodule (`sebllns/icov_core`): codec libraries, `icov_cli`, `icov_viewer`, tests, docs. Code work happens here.

## Where to look

- `icov-maintenance-plan.md`: rework plan toward network streaming. The "Status" table gives the current step; update it when a step changes state. Decisions D1 to D7 are listed under "Open decisions".
- `ARCHITECTURE.md`: internals, written for 1.6.0, partly outdated.
- `core/README.md`: build, run, CLI options, viewer keys.
- `core/doc/bitstream_report.md` (stream format, 2.x), `core/doc/streaming_protocol.md` (protocol spec), `core/doc/encoder_performance_report.md`.

## Build (in `core/`, conda env `icov-core`)

aff3ct is installed in `build/aff3ct-install` (rebuild only if missing, see `core/README.md` step 3).

```bash
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$CONDA_PREFIX;$PWD/build/aff3ct-install" \
  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
  -DBoost_USE_STATIC_LIBS=OFF -DICOV_USE_OMP=ON -DICOV_BUILD_TESTS=ON
cmake --build build/release -j"$(nproc)"
```

Debug: same with `-B build/debug -DCMAKE_BUILD_TYPE=Debug`. Without an activated env, prefix commands with `conda run -n icov-core`.

## Tests

```bash
ctest --test-dir build/release -L unit --output-on-failure         # about 1 s
ctest --test-dir build/release -L integration --output-on-failure  # about 20 s
```

- Unit tests after any change; integration tests after any change to the codec, stream format, extractor or decoder.
- The integration test encodes twice (outputs must be identical), decodes a path and checks block hashes (`--verify-hash`). Decoded blocks, rates and PSNR must stay identical unless the change is meant to alter them.

## Running

Binaries are in `core/build/release/bin`. Test image: `python scripts/gen_synthetic_erp.py erp.ppm` (from `core/`). Encoding 1024x512 takes about 80 s; `--ldpc-bp-ite 20` is about 3x faster. Headless decode of a path:

```bash
./icov_cli -m decode -i erp.icov --decode-positions-input positions.csv \
  --decode-csv-output <dir>/run --decode-headless 1 --verify-hash
```

Write temporary outputs to the session scratchpad, not into the repositories.

## Conventions

- C++17. Minimal diffs, follow the style of the surrounding code.
- Reports go in `core/doc/`: measured numbers with their setup, code references as function and file.
- Version in `core/version.txt`. Tags `vX.Y.Z` only on request.
- Core: one commit per step, feature branches for multi-step work. Superproject: after core commits, bump the submodule pointer with "Update core: ..." or "Update core to vX.Y.Z".
- Never push, tag, force-push or rewrite history without an explicit request.

## Do not read

- `core/data/imt_18/` (271 MB of text matrices, unused by the code).
- `core/nogit/` (private notes and conversation exports).
- `core/build/` except logs or outputs needed for the task.
