# ICOV

_Interactive Coder for Omnidirectional Videos, INRIA ADT project_

ICOV encodes an equirectangular (ERP) 360 image or video once. A client then decodes only the blocks of its viewport, and the transmitted rate depends on the blocks it has already decoded. For the principle, see https://ieeexplore.ieee.org/document/9171593 and https://project.inria.fr/intercom/.

This repository is the superproject: it pins the codec as a submodule and holds the demonstrators, network stubs and helper scripts around it. The codec itself, its build instructions, the CLI and the viewer live in [icov_core](https://github.com/sebllns/icov_core) (`core/`).

## Layout

| Path | Content |
|---|---|
| `core/` | Submodule [icov_core](https://github.com/sebllns/icov_core): codec libraries, `icov_cli`, `icov_viewer`, tests, documentation |
| `unity/icov_demonstrator/` | Unity demonstrator and its native rendering plugin |
| `unity/vlc/` | VLC Unity plugin and test project |
| `renderer/` | Client controller and renderer used by the Unity plugin |
| `network/` | Client and server interfaces (stubs, no codec data is transported yet) |
| `scripts/` | Python helpers: batch encoding, UDP remote control and monitoring of the player |
| `tools/` | Media helpers: EAC to ERP conversion, 360 metadata tag, video recoding, ERP to GIF |
| `tests/` | Legacy private tests (memory, parser, OpenMP, Varodayan LDPCA) and a Unity 360 player |
| `ARCHITECTURE.md` | Codec architecture and internals |
| `icov-maintenance-plan.md` | Rework plan toward interactive 360 video streaming |

## Getting the sources

```bash
git clone --recurse-submodules https://github.com/sebllns/icov.git
cd icov
# or, in an existing clone:
git submodule update --init --recursive
```

## Build

Build the codec from `core/`, following [core/README.md](core/README.md) (conda environment, aff3ct, CMake). This gives `icov_cli` and `icov_viewer`, enough to encode, decode and navigate.

The top-level `CMakeLists.txt` (Unity plugins, `-DICOV_BUILD_UNITY_PLUGINS=ON`) does not configure in the current version: it includes `.cmake/AddGitSubmodule.cmake`, which is missing. The Unity projects target an old Unity version and are kept for reference.

External tools used by `tools/`: `ffmpeg`, `exiftool`, Python with NumPy and SciPy.

## Documentation

- Codec usage: [core/README.md](core/README.md)
- Internals: [ARCHITECTURE.md](ARCHITECTURE.md)
- Doxygen: https://sebllns.github.io/icov_core/
- Demo video: https://vimeo.com/user188622130

## License

Copyright (C) 2021-2026 by Inria. All rights reserved.

Distributed under the GNU Affero General Public License, version 3 or later (see [LICENSE](LICENSE)), WITHOUT ANY WARRANTY. For use under terms that do not comply with the GNU AGPL, contact Inria about an ICOV Professional Edition License.

Developed at Inria Rennes - Bretagne Atlantique, Campus Universitaire de Beaulieu, 35042 Rennes Cedex, France.

## Contact

- Developer: BELLENOUS, Sébastien
- Project manager: MAUGEY, Thomas
- Team leader: ROUMY, Aline
