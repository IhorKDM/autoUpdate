"""Export the FastAPI frontend for GitHub Pages; no Python server runs on Pages."""
import json
import os
import shutil
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from web.main import STATIC, config

def build(destination):
    destination.mkdir(parents=True, exist_ok=True)
    shutil.copytree(STATIC, destination / "static", dirs_exist_ok=True)
    shutil.copyfile(STATIC / "index.html", destination / "index.html")
    (destination / "config.json").write_text(json.dumps(config()) + "\n")
    (destination / ".nojekyll").touch()

if __name__ == "__main__":
    build(Path(sys.argv[1] if len(sys.argv) > 1 else "site-dist"))
