# Text Echo

A complete learning project: C++ / Qt 6 desktop app, a Python / FastAPI website,
JavaScript + CSS frontend, GitHub Actions CI, tag-triggered desktop releases,
and startup update notifications.

The app has two text fields and two separate outputs. Click **Show text**, or
press Enter in either field, to display both values.

- **FastAPI website:** http://167.99.87.65/
- **Static mirror:** https://ihorkdm.github.io/autoUpdate/
- **Downloads:** https://github.com/IhorKDM/autoUpdate/releases/latest
- **Builds:** https://github.com/IhorKDM/autoUpdate/actions
- **Lesson:** [Step-by-step walkthrough](docs/WALKTHROUGH.md)

## Run locally

Qt 6.5+ (Widgets, Network, Test), CMake 3.21+, and a C++17 compiler are required.
On macOS with Homebrew Qt installed:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build --config Release
open build/TextEcho.app
```

For the Python website (Python 3.12 recommended):

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-dev.txt
.venv/bin/python -m uvicorn web.main:app --host 127.0.0.1 --port 8000
```

Visit http://127.0.0.1:8000. The interactive browser preview echoes your text;
the download buttons fetch the latest platform ZIPs from GitHub Releases.
FastAPI also serves `/health`, `/config.json`, `/download/macos`,
`/download/windows`, and API documentation at `/docs`.

Alternatively run `docker compose up --build -d`; stop with `docker compose down`.

## Test

```sh
ctest --test-dir build -C Release --output-on-failure
.venv/bin/python -m pytest -q
```

Desktop tests exercise the actual widgets, version comparison, rejected releases,
and an HTTP startup check. Backend tests check downloads, configuration, static
assets, and the exported frontend.

## What happens on GitHub

```text
Push / pull request → CI: Python tests + Qt builds/tests + Docker smoke test
                    ↓ (successful push to main)
               Deploy FastAPI to DigitalOcean + frontend to GitHub Pages

Push vX.Y.Z tag → validate version + tests → macOS + Windows build/test/package
                                           ↓ (both succeed)
                              publish release ZIPs + SHA-256 checksums
                                           ↓
                  website downloads and old apps discover the new version
```

- `.github/workflows/ci.yml`: continuous integration.
- `.github/workflows/release.yml`: tag-triggered release delivery.
- `.github/workflows/pages.yml`: frontend continuous deployment after successful CI.
- `.github/workflows/deploy-server.yml`: deploy the tested commit to DigitalOcean.
- `compose.production.yaml`: the server container, with health checks and image rollback.
- `render.yaml`: optional deployment of the **Python backend** on Render; needs a
  Render account. GitHub Pages serves the frontend only; it does not run FastAPI.

The same frontend runs on FastAPI and GitHub Pages. Pages uses a generated
`config.json`; FastAPI generates it from `RELEASE_REPOSITORY` (default:
`IhorKDM/autoUpdate`). Build frontend files with
`.venv/bin/python scripts/build_site.py site-dist`.

## Server deployment

The Python site runs on the DigitalOcean server at `http://167.99.87.65/`.
Successful CI on main triggers the deployment workflow, which SSHes as `ihor`
using a dedicated key restricted to the deployment command. The script only
accepts a commit SHA on main and deploys the exact tested revision. Failed
container health checks restore the previous image when one exists.

Source lives in `/home/ihor/text-echo/source` on the server. The forced command is
`/home/ihor/.local/lib/text-echo/deploy`, outside the Git checkout. GitHub stores
the dedicated key and verified host key as encrypted Actions secrets; no private
key is committed. Existing SSH credentials and services are preserved. The site
uses port 80 on the supplied IP; a custom domain and HTTPS can be added later.

See [server operations](docs/SERVER.md) for deployment and rollback commands.

## Release a new version

Change the version in `CMakeLists.txt`, commit, push, and tag that commit:

```sh
git add .
git commit -m "Release 0.1.3"
git push origin main
git tag -a v0.1.3 -m "Text Echo 0.1.3"
git push origin v0.1.3
```

Use your next unpublished version. Tags must match the CMake version exactly.
Do not move published tags. If packaging fails, fix it and publish a new version;
if only publication failed, inspect any existing draft release before retrying.

## Update behavior and platform scope

On launch (and when **Check for updates** is clicked), the app queries GitHub's
latest stable release. It requires a newer numeric version and the matching
platform asset. **Update — download new version** opens that release's ZIP in your
browser. Quit the old app and replace it with the extracted update.

**Installation is manual. This project does not implement in-place automatic
installation.** That requires a separate platform-specific updater and verified,
signed packages. Offline checks do not stop the app from working.

Downloads target macOS Apple Silicon and Windows x64. The macOS app is ad-hoc
signed for bundle integrity, without Developer ID signing or notarization;
Windows is unsigned. OS trust prompts are expected for these learning builds.
Windows users must extract the entire folder, including DLLs and plugins.

See [dependency notices](THIRD_PARTY_NOTICES.md) for Qt details.
