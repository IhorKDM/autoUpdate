# Walkthrough: from your code to someone's desktop

Use this after running the completed project. You don't need to configure a new
repository: this one is already at https://github.com/IhorKDM/autoUpdate .

## 1. Follow one input through Qt

Open `desktop/main.cpp`. It creates a `QApplication`, then a `Window`, then starts
Qt's event loop with `app.exec()`. The event loop receives clicks and key presses.

Open `desktop/window.cpp`. The constructor puts labels, a text field and buttons
in a `QVBoxLayout`. This connection defines what a click does:

```cpp
connect(show, &QPushButton::clicked, this, [entry, output] {
    output->setText(entry->text());
});
```

That is Qt's **signal and slot** system: a button emits a signal, and a callback
receives it. Return in the field also triggers the button. The output uses plain
text, so `<b>Hello</b>` is displayed literally instead of treated as HTML.

**Try:** change the Show text label, rebuild, and observe your change.

## 2. Follow one download through FastAPI

Run the Python server using the README commands. Visit http://127.0.0.1:8000/docs
and inspect `/download/{platform}`. With `platform=windows`, the handler returns
an HTTP redirect to:

```text
https://github.com/IhorKDM/autoUpdate/releases/latest/download/TextEcho-windows-x64.zip
```

GitHub resolves `latest` to a release and serves its asset. Your Python server
doesn't have to store the ZIP. The macOS route works the same way.

`web/static/index.html`, `style.css` and `app.js` implement the browser UI.
The browser demo uses `.textContent` to display your input. It is a preview of
the desktop interaction; it does not run C++ in the browser.

The same frontend is hosted at https://ihorkdm.github.io/autoUpdate/ . Pages
serves static files only. `scripts/build_site.py` exports the frontend and writes
`config.json`; this replaces the config endpoint when no Python server is present.

**Try:** open the page, enter text in the preview, and use a download button.

## 3. Read the CI workflow

Open `.github/workflows/ci.yml` and compare it with the Actions tab on GitHub.

- `on`: events that start automation: branch pushes and pull requests.
- `jobs`: independent groups of commands.
- `runs-on`: which computer runs those commands.
- `steps`: commands/actions executed in order within a job.
- `uses`: run a reusable action (checkout source, install Python, install Qt).
- `run`: run a command, just as you would in a terminal.
- `strategy.matrix`: repeat the desktop job on macOS and Windows.

The website job installs Python dependencies and runs pytest. The desktop jobs
compile and run CTest/Qt Test. The container job builds the FastAPI Docker image,
starts a container and waits for its health endpoint.

This is **continuous integration**: every proposed change is checked. A build on
your Mac cannot prove that the Windows build works, which is why we use two runners.

**Try:** make a deliberate failing assertion on a temporary branch and push it.
Inspect the failed step, then fix it. Do not push that change to a release tag.

## 4. Understand commits, tags, releases and artifacts

A **commit** is a snapshot of source code. A **tag** gives one snapshot a name:
`v0.1.0` identifies the source for that version. A **release** is the GitHub page
with notes and downloadable files for that tag. An **artifact** is a file produced
by automation; our temporary workflow artifacts later become release assets.

```text
commit abc123 ← tag v0.1.0
                    ↓ build
              release v0.1.0
               ├── macOS ZIP
               ├── Windows ZIP
               └── SHA256SUMS.txt
```

The version is declared once in `CMakeLists.txt`. CMake embeds it in the executable
and app bundle. The release workflow rejects a tag that doesn't match it.

Version numbers here use `MAJOR.MINOR.PATCH`. For this exercise use patch changes
for small fixes: `0.1.0`, `0.1.1`, `0.1.2`. Released versions must keep increasing.
Never move an existing published tag to a different commit.

## 5. Read the release workflow

Open `.github/workflows/release.yml`. Its trigger is `push.tags: ['v*']`.
A normal code push runs CI; a version-tag push runs the release workflow.

1. **validate** checks the exact version and runs the Python tests.
2. **package** builds and tests the desktop app on both platforms.
3. `macdeployqt` / `windeployqt` copy the Qt runtime libraries and plugins.
4. Both runners upload their ZIPs as workflow artifacts.
5. **publish** waits for both packages, downloads them, computes SHA-256 checksums,
   creates a draft with the assets, then makes the release public.

The `needs` keyword enforces that order. If either platform fails, publication
is skipped. Only the publish job has `contents: write`; GitHub provides its
short-lived `GITHUB_TOKEN`. No private token is embedded in downloads.

ZIPs must include dependencies. An `.exe` alone will not run on a machine without
Qt. On Windows, users must extract the whole folder, not just the executable.

This is **continuous delivery** for the desktop app: a tag causes checked packages
to be built and made available. It does not install new code on people's machines.

## 6. Read website CD

Open `.github/workflows/pages.yml`. Successful CI on `main` triggers it. It exports
the frontend, uploads a Pages artifact, and deploys it. The deployment job uses
`pages: write` and an OIDC identity token instead of a permanent deployment key.

This is **continuous deployment**: a successful code change updates the live
frontend automatically. Desktop releases and website deployment are distinct:
you can change the website without publishing a desktop version.

To host the actual Python server on Render, create a Blueprint from this repo
using `render.yaml`. It selects a free Python web service, health checks, and
automatic redeployment after repository checks pass. This requires your Render
account and is not the GitHub Pages deployment. Free services can sleep while idle.
Alternatively deploy the Dockerfile on your chosen Python/container host.

## 7. Follow the desktop update check

`Window::checkForUpdates()` asynchronously requests:

```text
GET https://api.github.com/repos/IhorKDM/autoUpdate/releases/latest
```

The desktop remains usable during the request. `desktop/release.cpp` parses the
result. It checks that:

- the release is stable and public;
- the tag has the expected `vMAJOR.MINOR.PATCH` format;
- the version is newer than the compiled app version;
- the matching platform ZIP exists;
- its URL points to the expected GitHub repository and release.

Numeric comparison matters: `0.1.10` is newer than `0.1.9`; string comparison can
get that wrong. We use `QVersionNumber`.

A newer release displays an update button. Clicking opens that exact version's
ZIP. The user quits the old app and replaces it. There is no automatic in-place
installation. GitHub API rate limits or an unavailable connection produce a retry
message; they do not stop the app's main function.

**Try:** launch version 0.1.0 after 0.1.1 is published. Then launch 0.1.1.
The first should offer an update; the second should report no newer compatible
release. Both release assets remain available from their tag pages.

## 8. Publish your next version

Make a small change, set the CMake version to the next unused value, and test:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.venv/bin/python -m pytest -q
```

For example, after 0.1.1:

```sh
git add .
git commit -m "Release 0.1.2"
git push origin main
# Check CI in GitHub Actions before pushing your release tag.
git tag -a v0.1.2 -m "Text Echo 0.1.2"
git push origin v0.1.2
```

Inspect Actions, the release assets, and the website download. Keep an old copy
of the app to test the update prompt. A release is successful when users can
actually download and run it, not just when a workflow starts.

## 9. Limits and the next lesson

These learning packages don't have trusted developer signatures or notarization.
The macOS bundle is ad-hoc signed for integrity. Windows packages are unsigned.
OS trust prompts are expected. Test on clean machines before wider distribution.
The macOS build supports Apple Silicon; the Windows build supports x64.

An in-place updater is a separate project: signed update metadata, verified
packages, an installer/helper outside the running app, rollback, and platform
permissions. A useful next lesson is integrating established platform updaters
and adding signing credentials as GitHub secrets.

## References

- GitHub Actions: https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax
- GitHub Pages workflows: https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages
- Qt deployment: https://doc.qt.io/qt-6/macos-deployment.html and https://doc.qt.io/qt-6/windows-deployment.html
- Render Blueprints: https://render.com/docs/blueprint-spec
