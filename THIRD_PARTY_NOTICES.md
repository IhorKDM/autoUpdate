# Third-party components

Text Echo uses Qt 6 (Core, GUI, Widgets, Network and their runtime plugins).
Qt is copyright The Qt Company Ltd. and other contributors, and is offered
under commercial and open-source licenses. This example uses dynamically
linked open-source Qt binaries. Qt component licenses and attribution:
https://doc.qt.io/qt-6/licensing.html

Qt open-source source archives, including the 6.8.3 release used in CI:
https://download.qt.io/archive/qt/6.8/6.8.3/single/

The desktop ZIPs include the LGPLv3 and GPLv3 license texts, Qt's GPL exception,
and this dependency notice. Qt's bundled dependency details are documented at
https://doc.qt.io/qt-6/licenses-used-in-qt.html . The app does not prevent replacing
its Qt shared libraries with compatible builds. Its CMake project and source
are available in the public repository associated with each release tag.

The FastAPI server uses FastAPI, Starlette, Pydantic and Uvicorn. Their license
information is included in the installed Python distributions. The server and
frontend are separate from the desktop ZIPs.

Website fonts: DM Sans and Space Grotesk, served by Google Fonts under the
SIL Open Font License. System sans-serif fonts are used if remote fonts are
unavailable.
