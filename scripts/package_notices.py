"""Include the project's dependency notices in each distributed desktop bundle."""
import shutil
import sys
from pathlib import Path
root = Path(__file__).resolve().parents[1]
destination = Path(sys.argv[1])
destination.mkdir(parents=True, exist_ok=True)
shutil.copyfile(root / 'THIRD_PARTY_NOTICES.md', destination / 'THIRD_PARTY_NOTICES.md')
shutil.copytree(root / 'licenses', destination / 'licenses', dirs_exist_ok=True)
(destination / 'INSTALL.txt').write_text(
    'Text Echo\n\n'
    'macOS: Extract the ZIP and move TextEcho.app to Applications. Apple Silicon only.\n'
    'Windows: Extract the entire ZIP to one folder, then launch TextEcho.exe.\n'
    'Keep the Qt DLLs and plugin folders alongside the executable.\n\n'
    'To update: download the new ZIP, quit the old app, then replace its folder/app.\n'
    'These learning builds are not signed with a trusted developer certificate.\n'
    'Source and releases: https://github.com/IhorKDM/autoUpdate\n'
)
