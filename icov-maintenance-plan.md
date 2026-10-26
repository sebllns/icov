# ICOV rework plan: interactive 360 video streaming

Target: interactive 360 video streaming over the network. A 360 image or video is encoded once and stored on a server. A client player sends its viewport (navigation) to the server, receives only the data needed for that viewport given what it has already decoded, decodes in real time, and shows user statistics and a developer debug overlay. Anything in the current prototype may be changed: data files, bitstream format, demonstrators, dependencies.

Repositories:
- `sebllns/icov_core` (1.6.0, last commit 2024-01-22): codec (LDPC/LDPCA layer, VTM-derived video layer, CLI, OpenGL/GLUT viewer, Qt demo).
- `sebllns/icov` (0.3): superproject with Unity demonstrators, renderer, network, scripts. `core/` submodule pinned to `icov_core@1ef2f6e`. Does not configure as is (includes a missing `.cmake/AddGitSubmodule.cmake`).

## Findings that drive the plan

| Area | Observation | Consequence |
|---|---|---|
| Tests/CI | One Boost.Test file, built only in Debug, no CTest. Only a GitLab Doxygen job. | Golden references and CI come first, before any rework. |
| Matrix data | `data/imt_18` = 271 MB: 1537 per-rate `H_<m>.alist` files (232 MB) plus `H.txt` (25 MB). Not referenced by name in `lib/` or `app/`. `data/imt_164_v2` = 104 KB: one base `H.pcm` plus combination matrices `C1x.pcm`, from which `IMT164MatrixProvider` derives the rate-specific codes. | The per-rate alists are probably legacy and derivable from a base matrix plus combination matrices (to verify). Target: tens of KB to a few MB in a binary format, no text alists. |
| aff3ct | Used for a sparse matrix type, a decoder base class, the SPA update rule, alist write, `hard_decide`. The LDPCA BP decoder itself is custom. Built at configure time with GSL, MKL, hwloc. | Removal is low risk. |
| VTM | Vendored pre-VTM JVET "NEXT" 1.1 snapshot (~45k LOC). Used for intra prediction, EMT transforms, quantization, RdCost, buffer/geometry types. | Prune first; replacement is coupled to the bitstream format redesign. |
| Build | Source-tree writes (version.h, version.txt), configure-time copy of `data/`, forced `-msse4.1`, hard-coded paths in scripts. | Fix during the build modernization step. |
| Demonstrators | Unity projects with committed binary assets and an old Unity version; VLC Unity plugin; GLUT viewer in `icov_core`. | Replace with a player built on a stable C API; Unity becomes an optional client. |
| Real-time feasibility | Unknown. Decoding is iterative BP on large LDPCA codes, per viewport and per frame. | Must be profiled before choosing the player architecture or the bitstream layout. |
| Network | `icov/network/` is empty stubs (client/server interfaces, ~150 lines total). `scripts/udp_*.py` only exchange stats/positions locally. Nothing transports codec data over a network today. | Networking is greenfield: protocol, server and client are new work. |
| Extraction semantics | `lib/extractor` (`InputIcov::selectPrediction`, `readSIRates`, `readSyndromesSizes`, `BlockExtractor` ordering loop) shows the stored file holds, per block, syndrome sizes for each candidate side information (prediction). For a request, the extractor picks the prediction available at the client (from the decoding order and already-decoded blocks) and reads the matching syndrome prefix. | Extraction depends on client state. Either the server tracks it (stateful) or the client computes what it needs from an index and fetches byte ranges (stateless). This is the central design choice for streaming. |

## Target architecture

```
  OFFLINE                         SERVER                                  CLIENT
  -------                         ------                                  ------
  source 360    icov-encode       icov-server                             icov-player
  image/video ----------------->  stores .icov v2 files                   viewport + navigation
                .icov v2 file     (+ index, matrices)                     |
                                  |                                       v
                                  |  <-- request: frame, viewport,   ---  request scheduler
                                  |      client state / byte ranges       (prediction, prefetch,
                                  |                                        rate control)
                                  |  --> syndrome chunks + params    -->  receive buffer
                                  |      (QUIC streams)                    |
                                  |                                       decode workers (libicov)
                                  |  <-- client stats (optional)          |
                                                                          render thread (GPU shader)
                                                                          stats + debug overlay

  All three components link libicov through the same C API.
  Python package wraps the C API for analysis, simulation and test clients.
```

Rationale:
- The codec value is in sending only what the client's current viewport needs, given the side information the client already holds. The system is therefore request-driven, not a pushed broadcast stream.
- One C API serves the encoder, the server, the player, Python and optional Unity/web clients. Unity version churn no longer touches the codec.
- On the client, network receive, decoding and rendering run on separate threads. The renderer never waits: it shows the last complete frame plus a fallback for regions not yet received or decoded.
- Rendering a viewport from an equirectangular frame is one fragment shader. The hard parts are request round-trip latency and decode time, not rendering.

### Streaming design choice: who performs extraction

| Model | How it works | Strengths | Weaknesses |
|---|---|---|---|
| A. Stateful server-side extraction | Client sends viewport and navigation events; server tracks what the client has decoded, runs the extractor, sends exactly the needed syndrome bits. | Closest to the current extractor; minimal bytes sent; server can do rate control. | Per-client server state and CPU; not cacheable by CDNs; server must mirror client state exactly (desync risk on loss/reconnect). |
| B. Stateless client-driven pull | File carries an index of syndrome-prefix offsets per block and per candidate side information. Client computes which prediction it can use and which byte ranges it needs, then fetches them (HTTP range requests or equivalent). | Scales like DASH: static files, HTTP caches/CDN; no server state; simple server. | Index size; more requests or larger merged ranges; only works if the needed rate is fully determined by data the client has. |
| C. Hybrid | Pull model by default; optional server hints (rate, viewport prediction) or server-side extraction for constrained clients. | Flexibility. | Two code paths to test. |

Step 0 and step 6 determine whether model B is possible, i.e. whether the rate needed for a block is fully determined by the index and by blocks the client already decoded. Recommendation: B (or C) if possible, because it decouples the codec from server scaling. A is the fallback and matches the current code.

### Transport options

| Option | Strengths | Weaknesses | Fit |
|---|---|---|---|
| QUIC (HTTP/3 or raw QUIC, e.g. msquic, quiche, ngtcp2) | Multiplexed streams without head-of-line blocking across tiles, 0-RTT/1-RTT setup, stream priorities and cancellation (useful when the viewport moves), connection migration | Library choice and TLS setup; UDP sometimes filtered | Recommended for the real system |
| HTTP/2 or HTTP/1.1 range requests over TCP | Simplest, works with any static server and CDN (model B) | Head-of-line blocking on loss; cancellation is coarse | Good first implementation of model B |
| WebTransport (over HTTP/3) | Browser support for QUIC streams and datagrams | Requires HTTP/3 server; browser-only API | For the web client |
| WebSocket / TCP custom protocol | Easy prototype for model A | Head-of-line blocking; no per-tile priority | Prototype only |
| WebRTC data channels | Browser, NAT traversal, low latency | Heavy stack; designed for peer-to-peer | Not needed unless P2P or browser-to-browser |

Recommendation: start with HTTP range requests (model B) or a TCP/WebSocket protocol (model A) to get an end-to-end loop working, then move to QUIC with one stream per tile/request and priorities from the viewport. Keep the transport behind an interface so both can be benchmarked.

### Latency and quality control on the client

- Motion-to-photon cannot include a network round-trip. Hide it with: a decoded margin around the visible FOV, viewport prediction (linear extrapolation first, learned predictors later), and prefetch of a low-rate version of the full sphere or of neighboring tiles.
- Request scheduler: per frame deadline, priority by angular distance to the predicted gaze, cancellation of stale requests when the viewport moves.
- Buffer: short (a few hundred ms) because interactivity matters more than smoothness; stall and freeze statistics are first-class metrics.
- Metrics: bitrate, bytes per displayed viewport, request latency (p50/p95), decode time, stall count and duration, viewport PSNR/WS-PSNR when the reference is available, fraction of viewport displayed at full quality.

### Player technology options

| Option | Strengths | Weaknesses | Fit |
|---|---|---|---|
| Native C++: SDL3 + OpenGL 3.3/4.1 + Dear ImGui | Small, portable, direct access to libicov, ImGui/ImPlot give stats panels and debug overlay with little code | macOS caps OpenGL at 4.1; no VR without extra work | Recommended baseline |
| Native C++: SDL3 GPU API or bgfx/sokol | One codebase over Vulkan, Metal, D3D12 | Extra abstraction; SDL3 GPU is recent (2025) | Alternative if Metal/Vulkan are needed |
| Raw Vulkan | Compute shaders for GPU-side decoding experiments | Large boilerplate for a textured sphere | Only if GPU LDPC decoding becomes a goal |
| OpenXR on top of the native player | Standard HMD API (Quest via Link, SteamVR, Monado) | Runtime dependency, test hardware | Phase 2 of the player |
| Python | Analysis tools, notebooks, simulated clients for network experiments | Not suited to real-time playback | Analysis and simulation, not the main player |
| Unity (current) | Mature XR support | Version churn, binary assets, C# interop, licensing | Optional client over the C API |
| Web (Emscripten + WebGPU/WebXR + WebTransport) | Zero-install demo, headset browsers, natural fit for HTTP/QUIC streaming | Decode speed in WASM, threading constraints | After native decode meets the target with headroom |

Recommendation: native C++ player first, OpenXR second, web and Unity as optional clients.

## Bitstream format v2 (requirements, to be specified before implementation)

The format serves two roles: storage on the server and the unit of network transfer. Both must be specified together.

- Header: magic, format version, codec version, projection (equirectangular, later cubemap), resolution, frame rate, GOP structure, matrix set identifier and its hash.
- Random access index: frame/GOP to offsets, then block/tile to syndrome-chunk offsets per candidate side information and per rate level. Its size and layout decide whether model B is practical (index per GOP, fetchable separately).
- Incremental rate chunks: syndromes ordered so that any rate is a prefix, enabling byte-range or stream-offset serving without re-encoding.
- Network packetization: chunk boundaries aligned with what a request returns; each transmitted unit self-describing (frame, block, prediction id, rate level, length, checksum) so the client can decode out of order and detect loss.
- Integrity: per-chunk checksum (CRC32C or xxHash64).
- Extensibility: tagged sections (TLV); embedded encoder parameters and seed for reproducibility.
- Container: custom chunked format vs ISO BMFF boxes (MP4 tooling, CMAF/DASH-style segments, existing HTTP infrastructure). Decide in step 6.
- Live encoding (encode while streaming) is a different requirement from on-demand; see decision D5.
- Compatibility with 1.6.0 may break; the 1.6.0 release stays the reference for previously published results.

## Matrix data

- Verify that the per-rate `imt_18` codes are derivable from `H` plus combination matrices, like `imt_164_v2`. If yes, store only the base and combination matrices.
- Binary format: small header (magic, version, n, m, edge count, hash) plus CSR/CSC index arrays (uint16 when n <= 65535, else uint32), compressed with zstd or packaged as `.tar.gz` with a SHA-256 manifest.
- Fallback when derivation is not possible: the same binary format per rate, compressed and fetched on demand as a release asset.
- The stream header references the matrix set by id and hash; client and server must hold the same set. Small sets can be embedded in the binaries; larger sets are downloaded once and cached by the client.

## Execution plan

| Step | Content | Objectives |
|---|---|---|
| 0 | Baseline build, golden references, determinism, profiling, extraction semantics | prerequisite |
| 1 | CI and tests | 1 |
| 2 | Release 1.6.0 as the archival prototype + Docker image | 2 |
| 3 | Build modernization, matrix data rework | 5 |
| 4 | Replace aff3ct | 3 |
| 5 | VTM audit and prune | 4 |
| 6 | Bitstream v2 and streaming protocol spec, then implementation | new |
| 7 | C API + Python package | 5 (Python) |
| 8 | Streaming server | new |
| 9 | Native streaming player with stats and debug overlay | new |
| 10 | Network experiments and performance (decode and transport) | new |
| 11 | Optional clients: OpenXR, web, Unity | new |

Steps 4, 5 and the decode-side part of 10 can run in parallel once step 0 provides golden tests and a benchmark. Steps 8 and 9 can start on a local (loopback) transport before the network layer is final.

Open decisions:
- D1. Dependency source for CI/Docker: decided, conda-forge (`environment.txt`, `environment-doc.txt` in core). The `sp/` superbuild and its Conan files were removed (tag `last-superbuild` in core). A conda lockfile can pin exact versions.
- D2. Container for bitstream v2: custom chunked format vs ISO BMFF.
- D3. Real-time target: resolution (e.g. 4K or 8K equirectangular), frame rate, viewport size, motion-to-photon budget, and network conditions (bandwidth, RTT, loss) to support.
- D4. Whether the superproject `icov` survives as a meta-repo or is replaced by separate repos (`icov_core`, `icov-server`, `icov-player`, optional `icov-unity`).
- D5. On-demand only (encode offline, stream stored files) or live (encode while streaming). Live changes encoder speed requirements and the index design. Plan assumes on-demand first.
- D6. Extraction model A, B or C (see above), decided from step 0 and step 6 findings.
- D7. Comparison baseline for evaluation: tiled viewport-adaptive streaming with a standard codec (e.g. HEVC/VVC tiles with DASH-SRD or OMAF viewport-dependent profiles), so results are comparable with the 360 streaming literature.

---

## Step 0. Baseline, golden references, profiling

Prompt for Claude Code (in `icov_core`):

```
Context: C++17 CMake project icov_core (interactive coder for omnidirectional video, LDPCA + VTM-derived intra coding). This task prepares the repo for a deep rework. No codec behavior change.

Tasks:
1. Build Release and Debug as the project is today; document working commands and dependency versions in docs/BUILDING.md. Report failures.
2. Deterministic end-to-end scenario with icov_cli: generate a small synthetic equirectangular clip (OpenCV or ffmpeg testsrc, fixed seed, 2-4 frames), encode at fixed QP with data/imt_164_v2, extract and decode 2-3 fixed viewports.
3. Golden outputs in tests/golden/: SHA-256 of bitstream(s) and decoded outputs, per-frame PSNR, rate per viewport. Commit the generator, not the video.
4. Determinism: run 3 times with and without ICOV_USE_OMP; report hash differences and likely causes, do not fix.
5. Profiling: for the decode path, measure wall time per viewport per frame, split into extraction, LDPCA decoding (BP iterations count), inverse transform/intra reconstruction, and color conversion. Use perf (or gprof if perf is unavailable). Report results for at least two resolutions, with machine, compiler, flags and thread count. Identify the top 10 hot functions.
6. Determine whether data/imt_18 is used anywhere (code, scripts, docs). Determine how IMT164MatrixProvider derives rate-specific matrices from H.pcm + C1x.pcm, and whether imt_18/<n>/alist/H_<m>.alist can be reproduced from imt_18/<n>/H.txt + C14.txt/C18.txt with the same procedure. Write a small check program, report how many files match exactly.
7. Extraction semantics (the target is network streaming, so this decides the protocol design). From lib/extractor (InputIcov::selectPrediction, readSIRates, readSyndromesSizes, BlockExtractor ordering loop), lib/decoder and the request_t type, document:
   - what a request contains (viewport, frame, ordering) and how the access/decoding order of blocks is derived from it,
   - how the prediction (side information) used for each block is chosen, and which client-side state it depends on (previously decoded blocks, previous frames, previous viewports),
   - what is stored per block per candidate prediction (syndrome sizes, rate levels, prediction parameters),
   - whether a client that knows the stored index and its own decoded state could compute exactly which bytes it needs (stateless pull) or whether information only available to the server is required (stateful extraction),
   - the bytes read by the extractor for each golden viewport versus the file size.

Output: docs/baseline_report.md with build status, determinism, golden hashes, profiling table, imt_18 findings, extraction semantics. Minimal diffs. Propose a commit message.
```

## Step 1. CI and tests (objective 1)

Prompt for Claude Code (in `icov_core`):

```
Context: icov_core, golden references in tests/golden/, Boost.Test in tests/unit_tests.

Tasks:
1. Decouple ICOV_BUILD_TESTS from CMAKE_BUILD_TYPE (default OFF). Register tests with CTest. Keep Boost.Test.
2. Unit tests: matrix loading (lib/icov/ldpc_codec_layer/io), BitSymbolProvider, LLRProvider, IMT164Syndrome round-trip at zero noise, LDPCA decoding on data/imt_164_v2/256 with known syndrome. Labels: unit, e2e.
3. E2E CTest comparing golden hashes and PSNR (stdlib-only Python 3 script or CMake script).
4. .github/workflows/ci.yml: push/PR/dispatch; ubuntu-24.04, gcc and clang, Release and Debug (Debug with ASan/UBSan); apt dependencies (list them); submodules recursive; ccache via actions/cache; ctest -L unit everywhere, -L e2e on Release gcc. Actions pinned, permissions: contents: read.
5. Non-blocking static checks job: clang-format --dry-run (add .clang-format matching current style, no mass reformat), cppcheck on lib/.
6. Port the GitLab Doxygen job to GitHub Pages.

If a test exposes a real bug, mark it expected-fail and report it. Minimal diffs. Propose a commit message.
```

## Step 2. Release 1.6.0 (archival prototype) and Docker image (objective 2)

Purpose: freeze the current prototype as a citable, reproducible reference before the format and architecture change.

Prompt for Claude Code (in `icov_core`):

```
Context: icov_core with green CI. Version hard-coded in CMakeLists.txt (1.6.0) and written into the source tree at configure time.

Tasks:
1. Version single source of truth: read version.txt in CMake, project(... VERSION ...), generate version.h with configure_file into the build tree, remove the committed version.h.
2. install() rules for libraries, icov_cli, headers (include/icov), data (share/icov/data).
3. Multi-stage Dockerfile: ubuntu:24.04 pinned by digest; builder runs ctest -L unit; runtime contains only runtime libs, non-root user, ENTRYPOINT icov_cli. Exclude data/imt_18 (build arg ICOV_WITH_FULL_DATA=false). OCI labels incl. licenses=AGPL-3.0-or-later. Add .dockerignore.
4. .github/workflows/release.yml on tag v*: check tag == version.txt; build and test; GitHub release with source tarball including submodules, SHA256SUMS, changelog excerpt; push image to ghcr.io/sebllns/icov_core with X.Y.Z, X.Y, latest; SBOM and provenance via buildx.
5. CHANGELOG.md (Keep a Changelog) with 1.6.0 entry marked as the last prototype-format release; CITATION.cff.

Do not create or push the tag. Give me the tagging commands. Minimal diffs. Propose a commit message.
```

## Step 3. Build modernization and matrix data (objective 5)

Prompt for Claude Code (in `icov_core`):

```
Context: directory-scoped CMake, GLOB_RECURSE sources, configure-time copy of data/ (271 MB of text alists in data/imt_18), forced -msse4.1. See docs/baseline_report.md for imt_18 findings.

Tasks, one commit each, golden tests passing after each:
1. Target-based CMake (target_include_directories/target_link_libraries, correct PUBLIC/PRIVATE), cmake_minimum_required 3.21, explicit source lists in lib/.
2. CMakePresets.json: dev-debug, dev-release, ci-gcc, ci-clang, docker.
3. -msse4.1 only on x86_64 behind option ICOV_ENABLE_SIMD; verify configuration for aarch64.
4. Binary matrix format:
   - Spec in docs/matrix_format.md: magic, version, n, m, edge count, index width, SHA-256 of payload, CSR row pointers + column indices, little-endian.
   - Reader/writer in lib/icov/ldpc_codec_layer/io with O(edges) load time and memory; mmap-friendly layout.
   - Converter tool (tools/icov_matrix_convert) from alist/pcm/txt to the binary format.
   - If step 0 showed imt_18 per-rate codes are derivable from base + combination matrices, store only those; otherwise convert all per-rate files and package them as a compressed archive (zstd or tar.gz) with a manifest.
   - Test: loaded binary matrices are identical to the text originals.
5. Runtime data lookup: CLI flag, env ICOV_DATA_DIR, install prefix share/icov/data, build tree. Stop copying data/ at configure time. scripts/fetch_data.py (stdlib only, SHA-256 check) to download archived matrix sets from release assets.
6. Remove text matrices from the default tree once the binary path is validated (separate commit, I will review before history rewriting is even considered).
7. Report clean and warm build times, repository and Docker image sizes before/after.

Minimal diffs. Propose one commit message per commit.
```

## Step 4. Replace aff3ct (objective 3)

Background: LDPCA codes (Varodayan, Aaron, Girod, "Rate-adaptive codes for distributed source coding", Signal Processing 86(11), 2006); aff3ct (Cassagne et al., SoftwareX 10, 2019). The superproject's `tests/varodayan_test/ldpca.cpp` can serve as an independent oracle.

Prompt for Claude Code (in `icov_core`):

```
Context: aff3ct (third-party/aff3ct submodule, built at configure time with GSL/MKL/hwloc) is used for: tools::Sparse_matrix, module::Decoder_LDPC_BP/Decoder_SISO base classes of Decoder_LDPCA_BP_flooding_SPA, tools::Update_rule_SPA, tools::AList::write, tools::hard_decide, tools::runtime_error, tools::version. Verify with grep first.

Phase A: docs/aff3ct_removal.md with each symbol, its exact semantics (SPA numerics: tanh/atanh form, clamping, float32, operation order in the flooding schedule), and the replacement design. Stop for review.

Phase B:
1. Sparse binary matrix with CSR+CSC adjacency in contiguous arrays, built on the binary matrix reader from step 3. Document complexity.
2. Self-contained LDPCA BP flooding SPA decoder implementing ILDPC_Decoder, same message schedule and float operation order. Design the hot loop for later SIMD and multithreading (structure-of-arrays messages, no per-iteration allocation).
3. Keep aff3ct behind ICOV_WITH_AFF3CT during transition; equivalence test over all matrices in data/imt_164_v2 and data/varodayan with seeded random syndromes and LLRs: identical decoded bits and iteration counts.
4. Benchmark both (time per codeword, peak RSS) on 256/1024/4096; report with machine/compiler/flags.
5. Default OFF once equivalent; final separate commit removes the submodule, GSL/MKL/hwloc lookups and .cmake/install_aff3ct.cmake.

No new dependencies. If bit-identity is impossible, report the LLR divergence and its effect on decoding success and rate, then stop. Propose commit messages.
```

## Step 5. VTM audit and prune (objective 4)

Prompt for Claude Code (in `icov_core`):

```
Context: third-party/vvc vendors a pre-VTM JVET NEXT 1.1 snapshot (~45k LOC). lib/icov/video_layer uses IntraPrediction, TrQuant_EMT, Rom, Quant, RdCost/RdCostWeightPrediction, and PelBuf, CPelBuf, PelUnitBuf, UnitArea, CompArea, TransformUnit, QpParam. app/icov_viewer/encoder/FakeEncoder.cpp includes CommonLib/Unit.h. The bitstream format will be redesigned in a later step, so bit-exactness with 1.6.0 is required only for this step.

Phase A: docs/vtm_usage.md: transitive set of needed VTM files (confirm with --gc-sections/--print-gc-sections or nm), purpose of each component in ICOV, LOC, SIMD usage, and cost estimates for (a) prune, (b) prune + ICOV-owned buffer/geometry types, (c) full reimplementation of intra prediction, transforms and quantization. Also list which of these components dominate decode time according to docs/baseline_report.md. Stop for review.

Phase B (option (a) unless told otherwise): move needed files to third-party/vtm_subset/ with original BSD headers and a README (upstream version, modifications), delete the rest, SIMD only behind ICOV_ENABLE_SIMD. Golden tests bit-exact. Report LOC and build time before/after. Propose commit messages.
```

## Step 6. Bitstream v2 and streaming protocol

Prompt for Claude Code (in `icov_core`):

```
Context: icov_core after aff3ct removal and VTM pruning. Final target: interactive 360 video streaming over the network. A file is encoded once and stored on a server; a client sends viewport/navigation information and receives only the data needed for its viewport given what it has already decoded. docs/baseline_report.md section "Extraction semantics" describes how the current extractor selects data per block and per candidate side information. The current layout (lib/encoder, lib/extractor, lib/icov/core/video_layer/nal, app/icov_cli/modes) will be replaced.

Phase A (specs only, two documents):

docs/bitstream_v2.md
- current layout as reverse-engineered from the code,
- v2 requirements: header (magic, format/codec versions, projection, resolution, fps, GOP, matrix set id + hash, encoder parameters and seed), random-access index (frame/GOP -> offsets; block -> syndrome-chunk offsets per candidate side information and per rate level), incremental rate chunks (any rate is a prefix), per-chunk checksum (CRC32C or xxHash64), TLV sections,
- index size estimate for 4K and 8K equirectangular at the current block size, and whether the index must be split per GOP,
- custom chunked format vs ISO BMFF/CMAF segments (tooling, HTTP/CDN compatibility, complexity),
- still-image mode, and what changes if live encoding is required later.

docs/streaming_protocol.md
- extraction model: A (stateful server-side extraction), B (stateless client pull of byte ranges from the index), or C (hybrid). State whether B is feasible given the extraction semantics, and the consequences of each for server state, scalability, caching, loss and reconnect handling,
- messages: session setup (content id, matrix set, capabilities), viewport/navigation requests with deadlines and priorities, data units (self-describing: frame, block, prediction id, rate level, length, checksum), cancellation, acknowledgements or client state resync for model A, statistics reports,
- transport mapping: first implementation (HTTP range requests for B, or TCP/WebSocket for A) and target implementation (QUIC with one stream per request or per tile, priorities, cancellation), with the abstraction that lets both coexist,
- loss and timeout behavior: what the client displays when a block misses its deadline, how a late block is used or dropped,
- security: TLS, no unauthenticated server-side file paths, request size limits.
Stop for review.

Phase B (after approval):
1. Reader/writer library for v2; encoder writing v2; extractor reading v2.
2. Protocol message definitions with a versioned, explicit binary encoding (hand-written little-endian structs or a schema such as FlatBuffers; justify the choice, minimize dependencies), plus encode/decode tests and a libFuzzer target for message and file parsing (nightly CI).
3. icov_cli subcommands: encode, info, verify, extract (simulates a server answering a request sequence), decode.
4. Loopback end-to-end test: a request sequence from a recorded viewport trajectory, extraction, decode, compare with golden v2 hashes.
5. Optional 1.6.0 -> v2 converter. Keep 1.6.0 golden tests on the archived path.
Propose commit messages.
```

## Step 7. C API and Python package

Prompt for Claude Code (in `icov_core`):

```
Context: icov_core with bitstream v2 and protocol messages. Clients of the API: encoder CLI, streaming server, native streaming player, Python package (analysis and simulated clients), optional Unity plugin and WASM build.

Phase A: docs/c_api.md proposing a C ABI (include/icov/icov.h):
- opaque handles: icov_encoder, icov_content (server-side opened file + index), icov_extractor (per-session state if model A), icov_session_client (client-side state: decoded blocks, side information), icov_decoder,
- data flow on the client: request planning (given viewport and client state, produce requests), ingest of received data units in any order, decode with deadline, frame output,
- error codes plus last-error string, explicit ownership, versioned struct sizes, thread-safety rules (which handles may be used concurrently),
- frame output as caller-provided or library-owned buffers (RGBA8 or YUV planes), viewport description (yaw, pitch, roll, FOV, output size),
- per-frame statistics (bytes requested/received, rate per block, BP iterations, decode time per stage, missed deadlines, PSNR when reference available) and debug data (block map, rate map, decode success map, side information used per block).
Python surface on top of it. Stop for review.

Phase B:
1. C API implementation, CMake target icov::icov_c, hidden symbol visibility by default, ABI check in CI.
2. Python package in python/: nanobind + scikit-build-core, requires-python >= 3.11, NumPy zero-copy frames, GIL released in long calls, .pyi stubs, py.typed, logging not print.
3. pytest round-trip and loopback-streaming tests against golden v2 hashes.
4. cibuildwheel for manylinux_2_28 x86_64.
5. Analysis helpers (extra 'icov[analysis]', matplotlib): rate and decode-success maps per viewport, bytes/latency/quality vs trajectory.
6. A simulated client in Python that replays a trajectory against the extractor with configurable bandwidth/RTT/loss (pure simulation, no sockets), producing per-frame JSONL statistics. Used for fast protocol experiments.
7. Port scripts/encode.py and scripts/decoder.py from the superproject as examples, no hard-coded paths.

Runtime dependency: numpy only. Propose commit messages.
```

## Step 8. Streaming server

Prompt for Claude Code (new repo `icov-server` or `apps/server` in `icov_core`, per decision D4):

```
Context: libicov C API, bitstream v2, docs/streaming_protocol.md with the chosen extraction model (A, B or C). Goal: a server that streams stored .icov files to interactive clients.

Requirements:
1. Transport behind an interface. First implementation per the spec (HTTP range requests for model B, or TCP/WebSocket for model A). Second implementation: QUIC (choose among msquic, quiche, ngtcp2 based on license, build complexity, maintenance and stream-priority support; document the choice).
2. Content catalog: content id -> file + index + matrix set; files opened read-only, memory-mapped; no client-controlled paths.
3. Model A only: per-session extractor state, bounded memory per session, session timeout and resync on reconnect.
4. Request handling with deadlines and priorities; cancellation of stale requests; per-connection send pacing.
5. Observability: structured logs (JSONL), per-session metrics (bytes sent, requests, cancelled, latency), Prometheus endpoint optional.
6. Limits: max sessions, max request size, rate limiting; TLS via a config option (QUIC requires it).
7. Docker image (multi-stage, non-root), config file, health endpoint.
8. Tests: unit tests for request parsing and scheduling; loopback integration test with a scripted client reproducing golden decode hashes; load test with N simulated clients reporting CPU and memory per session.
For model B, also document how to serve the same files from a plain HTTP server or CDN, and test it with a stock server (e.g. nginx or caddy) in CI.
Propose commit messages.
```

## Step 9. Native streaming player

Prompt for Claude Code (new repo `icov-player` or `apps/player`, per D4):

```
Context: libicov C API (client-side session, request planning, data ingest, decode), server from step 8, protocol in docs/streaming_protocol.md. Goal: interactive 360 streaming player with real-time decoding, user statistics and a developer debug overlay.

Stack: C++17, CMake, SDL3 (window, input, gamepad), OpenGL 3.3 core (4.1 max on macOS), Dear ImGui + ImPlot, glm. Dependencies via CMake FetchContent pinned to tags or a vcpkg manifest with baseline; justify the choice. Same transport interface as the server (start with the first transport, then QUIC).

Requirements:
1. Threads: render/input thread at display rate; network thread(s); decode worker pool; communication through bounded queues. The render thread never blocks on network or decode.
2. Request scheduler: per-frame deadlines; priority by angular distance to the predicted viewport; margin around the FOV (UI parameter); viewport prediction (linear extrapolation first, pluggable interface); cancellation of stale requests; optional low-rate full-sphere prefetch.
3. Rendering: equirectangular texture sampled from view direction in a fragment shader; blocks not yet decoded shown from the previous frame or the low-rate layer, with a visual marker in debug mode.
4. User statistics panel: fps, bitrate, request latency p50/p95, decode latency p50/p95, stall count/duration, fraction of viewport at full quality.
5. Developer debug overlay (F1): block grid on the sphere, per-block rate heat map, received/decoded/missed-deadline map, side information used per block, BP iterations, per-stage timing and network timeline charts (ImPlot), log console. Overlays as shader passes.
6. Trajectory record/replay (timestamped yaw/pitch/FOV, JSONL) and a headless benchmark mode (no window) that streams from a server or a local file and writes per-frame JSONL statistics.
7. Local mode (no network) reading a .icov file directly through the same request/ingest path, so codec and network issues can be separated.
8. CI: build on Linux (macOS if feasible); headless loopback test against the server with golden content.
Out of scope: OpenXR, web, Unity. Propose commit messages.
```

## Step 10. Network experiments and performance

Prompt for Claude Code (in `icov_core`, using server and player headless modes):

```
Context: server (step 8), headless player (step 9), Python simulated client (step 7). Targets from decisions D3 (resolution, fps, motion-to-photon, network conditions): <fill in>.

Tasks:
1. Experiment harness: Docker Compose (server + N headless clients) with network emulation via tc netem (bandwidth, delay, jitter, loss profiles defined in a versioned YAML file). Every run records git SHAs, container digests, seeds, trajectory files and netem profile, and writes JSONL results.
2. Viewport trajectories: synthetic (constant pan, fast head turns) plus public 360 head-movement datasets (check licenses; do not redistribute data if not allowed, provide a download script).
3. Metrics per run: bytes per displayed viewport, bitrate, request and decode latency distributions, stalls, missed deadlines, fraction of viewport at full quality, viewport PSNR/WS-PSNR when the reference is available.
4. Decode performance: thread-pool parallelism over independent blocks with deterministic output; SIMD for the LDPCA BP decoder (structure-of-arrays, AVX2 and NEON, keep the exact SPA path as reference); early termination on syndrome check. Report scaling 1-N threads.
5. Numerics-changing decoder variants (normalized/offset min-sum, layered scheduling): measure effect on decoding success, rate and speed; do not make any default without my decision. Cite references (Chen and Fossorier for normalized min-sum; Hocevar 2004 for layered decoding).
6. Transport comparison: first transport vs QUIC under the same netem profiles.
7. Baseline comparison (decision D7): viewport-dependent tiled streaming with a standard codec under the same trajectories and network profiles; document the exact baseline configuration.
8. Optional study only: GPU BP decoding feasibility (compute shaders or CUDA), rough estimate before any implementation.

All numbers reported with hardware, compiler, flags, threads, netem profile. Results reproducible from a single command. Propose commit messages.
```

## Step 11. Optional clients

- OpenXR in the native player: stereo rendering of the same texture; head pose drives viewport requests and prediction. Test on Monado (Linux) and SteamVR/Quest Link.
- Web: Emscripten build of the client-side libicov + WebGPU/WebXR renderer + WebTransport (or fetch range requests for model B). Only once native decode meets the target with headroom, since WASM is slower and threading needs SharedArrayBuffer (cross-origin isolation headers).
- Unity: thin native plugin over the C API plus a sample scene, in a separate repo; current Unity projects archived (tag) and removed from the main tree.

## Additional proposals

1. Determinism contract: same version + input + parameters produce bit-identical bitstreams and bit-identical decoded frames for the same received data; enforced by golden tests; codec, format and protocol versions carried in headers.
2. Rate-distortion regression: scheduled CI job computing BD-rate (Bjøntegaard, VCEG-M33, 2001) against the previous release.
3. Protocol version negotiation from the first release of the server, so client and server can evolve independently.
4. Licensing: AGPL-3.0 section 13 requires offering source to users interacting with a modified version over a network; the server and player should expose a source link (e.g. in session setup and the player's About panel). REUSE/SPDX headers; preserve VTM (BSD-3-Clause) notices in releases and images.
5. Security: fuzz all parsers that see network input (protocol messages, file index), run the server unprivileged in containers, TLS on by default outside localhost.
6. Zenodo DOI per release, CITATION.cff; experiment harness outputs suitable as paper supplementary material.
7. Pre-commit: clang-format, cmake-format, ruff.
8. Archive the superproject state (tag) before removing Unity assets; move remaining binary assets to Git LFS or out of the repo.
