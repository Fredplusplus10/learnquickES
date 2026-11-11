"""Writes the notes of a release, and refuses a tag the repository is not ready for.

Run from any directory: python3 scripts/release_notes.py v0.4.6

The notes are docs/RELEASE_NOTES_TEMPLATE.md with the version filled in and "What changed in this version."
replaced by that version's section of CHANGELOG.md. SECURITY.md ("Unsigned downloads") says why every release
carries the rest of the template. The tag must be "v" followed by the version in desktop/CMakeLists.txt, and
the changelog must have a section for it; otherwise nothing is printed and the exit code is 1.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PLACEHOLDER = "What changed in this version."


def fail(why: str) -> None:
    print(f"release_notes: {why}", file=sys.stderr)
    sys.exit(1)


def app_version() -> str:
    cmake = (ROOT / "desktop" / "CMakeLists.txt").read_text(encoding="utf-8")
    found = re.search(r"project\(\s*LearnQuick\s+VERSION\s+(\d+\.\d+\.\d+)", cmake)
    if not found:
        fail("desktop/CMakeLists.txt does not give the project a version like 1.2.3")
    return found.group(1)


def repository_url() -> str:
    """The public repository, from the one place in the code that names it."""
    header = (ROOT / "desktop" / "core" / "include" / "lq" / "release.h").read_text(encoding="utf-8")
    owner = re.search(r'kOwner\s*=\s*"([^"]+)"', header)
    repo = re.search(r'kRepo\s*=\s*"([^"]+)"', header)
    if not owner or not repo:
        fail("desktop/core/include/lq/release.h does not name the public repository")
    return f"https://github.com/{owner.group(1)}/{repo.group(1)}"


def changes(version: str) -> str:
    """The body of the changelog's "## <version> ..." section, without its heading."""
    lines = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8").splitlines()
    heading = re.compile(rf"^## {re.escape(version)}(\s|$)")
    start = next((i for i, line in enumerate(lines) if heading.match(line)), None)
    if start is None:
        fail(f"CHANGELOG.md has no section that starts with '## {version}'")
    end = next((i for i in range(start + 1, len(lines)) if lines[i].startswith("## ")), len(lines))
    body = "\n".join(lines[start + 1:end]).strip()
    if not body:
        fail(f"the section '## {version}' of CHANGELOG.md is empty")
    # The changelog links to files beside it; on a release page those links need the whole address, at this tag.
    files = f"{repository_url()}/blob/v{version}/"
    return re.sub(r"\]\((?!https?://|#)([^)]+)\)", lambda m: f"]({files}{m.group(1)})", body)


def notes(tag: str) -> str:
    version = app_version()
    if tag != f"v{version}":
        fail(f"the tag is {tag}, but desktop/CMakeLists.txt says the app is version {version} (expected v{version})")
    template = (ROOT / "docs" / "RELEASE_NOTES_TEMPLATE.md").read_text(encoding="utf-8")
    # Everything above the rule explains the template; the notes are what follows it.
    _, rule, text = template.partition("\n---\n")
    if not rule or PLACEHOLDER not in text or "X.Y.Z" not in text:
        fail("docs/RELEASE_NOTES_TEMPLATE.md no longer has its rule, 'X.Y.Z' and the line to replace")
    return text.replace("X.Y.Z", version).replace(PLACEHOLDER, changes(version)).strip() + "\n"


if __name__ == "__main__":
    if len(sys.argv) != 2:
        fail("usage: release_notes.py <tag>, for example v0.4.6")
    sys.stdout.reconfigure(encoding="utf-8", newline="\n")
    sys.stdout.write(notes(sys.argv[1]))
