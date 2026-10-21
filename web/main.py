"""The same download page is served by FastAPI and published to GitHub Pages."""
import os
import re
from pathlib import Path
from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles

app = FastAPI(title="Text Echo downloads", version="1.0.0")
STATIC = Path(__file__).parent / "static"
app.mount("/static", StaticFiles(directory=STATIC), name="static")
ASSETS = {"macos": "TextEcho-macos-arm64.zip", "windows": "TextEcho-windows-x64.zip"}

def repository():
    repo = os.getenv("RELEASE_REPOSITORY", "IhorKDM/autoUpdate")
    if not re.fullmatch(r"[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?/[A-Za-z0-9_.-]+", repo) or repo.rsplit("/", 1)[-1] in {".", ".."}:
        raise HTTPException(503, "Set RELEASE_REPOSITORY to your public GitHub owner/repository")
    return repo

@app.get("/", include_in_schema=False)
def index():
    return FileResponse(STATIC / "index.html")

@app.get("/health")
def health():
    return {"status": "ok"}

@app.get("/config.json")
def config():
    return {"repository": repository(), "assets": ASSETS}

@app.get("/download/{platform}")
def download(platform: str):
    if platform not in ASSETS:
        raise HTTPException(404, "Unsupported platform")
    return RedirectResponse(f"https://github.com/{repository()}/releases/latest/download/{ASSETS[platform]}")
