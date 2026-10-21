# Text Echo: learn releases with Qt 6 + FastAPI

This small project connects a desktop app to GitHub Releases. The Qt app echoes your input; the FastAPI site offers platform downloads. On startup, the app checks for a newer release and offers a button that opens its download page.

## 1. Run the desktop app

Requires CMake 3.21+, a C++17 compiler, and Qt 6.5+ Widgets and Network.

On this Mac (Qt is already installed):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt
cmake --build build --config Release
open build/TextEcho.app
```

Enter a value and click **Show text**, or press Enter. The output is plain text, including input containing HTML.

## 2. Run the website

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements-dev.txt
export RELEASE_REPOSITORY=YOUR_USERNAME/YOUR_REPOSITORY
python -m uvicorn web.main:app --reload
```

Open http://127.0.0.1:8000. Run tests with `python -m pytest -q`.
Replace the repository placeholder before running. Use a public GitHub repository for this example. No GitHub token belongs in the desktop app or frontend.

## 3. Understand the vocabulary

- **Commit**: a saved snapshot of your source code.
- **Tag**: a name attached to one commit, for example `v0.1.0`. It identifies which code was released. A tag itself contains no compiled app.
- **GitHub Actions**: GitHub's automation system. Workflow YAML files live in `.github/workflows/`.
- **Runner**: the machine executing a workflow. We use macOS to build the Mac app and Windows to build the Windows app.
- **Job**: a group of steps on one runner. Jobs can run in parallel; `needs` makes a job wait for successful dependencies.
- **CI (continuous integration)**: automatically check changes. `ci.yml` tests the web backend and compiles the desktop app on both platforms for branch pushes and pull requests.
- **Artifact**: a file saved from a workflow, such as a packaged ZIP. Workflow artifacts are intermediate files here; users download the assets attached to a GitHub Release.
- **CD (continuous delivery/deployment)**: automate making checked builds available. `release.yml` publishes desktop downloads when you push a version tag. This project implements desktop release delivery; it does not deploy the FastAPI server.
- **Release**: a user-facing GitHub page attached to a tag, with notes and downloadable assets.

## 4. Put the project on GitHub

Create an empty public GitHub repository (do not initialize it with a README). Then, from this folder:

```sh
git init -b main
git add .
git commit -m "Add Qt app, FastAPI site, and release workflows"
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPOSITORY.git
git push -u origin main
```

Open the repository's **Actions** tab. The CI workflow should run. A green result means the web tests passed and both desktop targets compiled; it does not prove that the packaged app runs on every user's machine.

Read `ci.yml` first: `on` selects the triggering event; `runs-on` selects a runner; `uses` runs an action; `run` runs a shell command. `strategy.matrix` repeats the same job on both operating systems.

## 5. Publish the first release

The version in `CMakeLists.txt` is `0.1.0`. The tag must match it:

```sh
git tag -a v0.1.0 -m "First release"
git push origin v0.1.0
```

`release.yml` performs these steps:

1. Validate the tag against the version and run the web tests.
2. Build the app on macOS and Windows, embedding the repository name and version.
3. Bundle Qt runtime dependencies with `macdeployqt` / `windeployqt`.
4. Upload both ZIPs as workflow artifacts.
5. Download both artifacts into the publication job, create a draft GitHub Release with its assets, then publish it.

Only the publication job receives `contents: write`. The temporary `GITHUB_TOKEN` is supplied by GitHub, not stored in the project. Both builds must succeed before publication. If publication fails after a draft is created, inspect that draft and finish or delete it before rerunning the workflow.

Assets have stable names: `TextEcho-macos-arm64.zip` and `TextEcho-windows-x64.zip`. Keep those names synchronized with `web/main.py`. Mac downloads target Apple Silicon; Windows downloads target x64. Intel Macs need an additional build or a universal build.

## 6. Publish an update and see the prompt

Install or launch the released **0.1.0** app. Then change the source as desired and change the CMake project version to **0.1.1**:

```sh
git add .
git commit -m "Release 0.1.1"
git push origin main
git tag -a v0.1.1 -m "Text Echo 0.1.1"
git push origin v0.1.1
```

Wait for the new release to publish, then reopen the old 0.1.0 app. It requests:

```text
GET https://api.github.com/repos/OWNER/REPO/releases/latest
```

The app compares the numeric release tag with its compiled version using `QVersionNumber`. Thus 0.1.10 is newer than 0.1.9. This learning project uses stable `vMAJOR.MINOR.PATCH` releases only. Do not move old release tags or mark an older release as latest.

If newer, the app shows **New version available** and an **Update — download new version** button. Clicking opens GitHub Releases; the person downloads, closes the old app, and replaces it. The website redirects to GitHub's latest-release asset URLs, so it needs no code changes per release. The first release must exist before download links work.

For a local build with update checks enabled:

```sh
cmake -S . -B build -DRELEASE_REPOSITORY=YOUR_USERNAME/YOUR_REPOSITORY
cmake --build build --config Release
```

Offline errors and API limits do not prevent normal use. The app tries again next launch. GitHub's public API is rate limited; large deployments should use cached release metadata on their own server.

## 7. What a fully automatic updater adds

This example detects updates and opens a download; it does not replace its own executable. A production updater needs signed update metadata, package verification, an external installer/helper, handling for running files and permissions, and rollback on failure. macOS and Windows need different installation strategies. A useful later lesson is integrating an established updater per platform.

The example packages are unsigned. For general distribution, add macOS Developer ID signing and notarization, and Windows code signing, before publishing. Also validate the packaged downloads on clean machines and include required Qt license notices. Windows users may need the matching Microsoft Visual C++ runtime if it is not available from packaging.

## 8. Deploying the website is a separate pipeline

A Python host runs `python -m uvicorn web.main:app --host 0.0.0.0 --port PORT`, where `PORT` is the actual port your provider specifies. Set `RELEASE_REPOSITORY` in its environment and install `requirements.txt`. Configure HTTPS and provider-specific deployment credentials there. The host can redeploy from `main`; that deployment is independent from releasing desktop versions. A concrete website CD workflow depends on which Python hosting provider you choose.

## References

- GitHub Actions syntax: https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax
- Qt macOS deployment: https://doc.qt.io/qt-6/macos-deployment.html
- Qt Windows deployment: https://doc.qt.io/qt-6/windows-deployment.html
- Qt setup action: https://github.com/jurplel/install-qt-action
