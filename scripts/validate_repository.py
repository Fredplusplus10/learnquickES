"""Dependency-free checks for LearnQuick repository layout and media references.

Run from any directory: python3 scripts/validate_repository.py
"""
from __future__ import annotations

import re
import sys
import xml.etree.ElementTree as ET
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]

# Properties every Qt Quick item already has. Qt marks them FINAL, and a QML component that declares one of
# them again is refused when it loads, together with every screen that uses it. The build does not notice:
# QML is loaded at run time. See "Fallos conocidos" in desktop/README.md.
ITEM_PROPERTIES = frozenset({
    "parent", "data", "resources", "children", "x", "y", "z", "width", "height", "opacity", "enabled",
    "visible", "palette", "visibleChildren", "states", "transitions", "state", "childrenRect", "anchors",
    "left", "right", "horizontalCenter", "top", "bottom", "verticalCenter", "baseline", "baselineOffset",
    "clip", "focus", "activeFocus", "activeFocusOnTab", "focusPolicy", "rotation", "scale", "transformOrigin",
    "transformOriginPoint", "transform", "smooth", "antialiasing", "implicitWidth", "implicitHeight",
    "containmentMask", "layer",
})
QML_PROPERTY = re.compile(r"^[ \t]*(?:(?:readonly|required|default)[ \t]+)*property[ \t]+[\w.<>]+[ \t]+(\w+)", re.M)
REQUIRED = (
    "README.md",
    "LearnQuick.html",
    "app/index.html",
    "app/css/kit.css",
    "app/css/app.css",
    "app/js/app.js",
    "tests/core.test.js",
    "tests/content.test.js",
    "docs/STUDENT-GUIDE.md",
    "desktop/CMakeLists.txt",
    "desktop/README.md",
    "desktop/Main.qml",
    "desktop/src/main.cpp",
    "desktop/Welcome.qml",
    "desktop/TodayPage.qml",
    "desktop/Shell.qml",
    "desktop/LessonPage.qml",
    "desktop/ExerciseSession.qml",
    "desktop/src/learnercontroller.cpp",
    "desktop/src/sessionmodel.cpp",
    "desktop/src/speechcontroller.cpp",
    "desktop/core/CMakeLists.txt",
    "desktop/storage/CMakeLists.txt",
    "desktop/tests/CMakeLists.txt",
    "desktop/content/course.json",
    "desktop/tests/data/web_reference.json",
    "assets/preview/github-social-preview.png",
    "assets/preview/readme-header.png",
    "assets/brand/learnquick-symbol.svg",
    "assets/brand/learnquick-wordmark.svg",
)
errors: list[str] = []


def ensure_file(path: Path, context: str) -> None:
    if not path.is_file():
        errors.append(f"MISSING: {context}: {path.relative_to(ROOT) if path.is_relative_to(ROOT) else path}")


def local_path(raw: str, base: Path) -> Path | None:
    if not raw or raw.startswith(("#", "data:", "mailto:", "tel:", "javascript:", "//")):
        return None
    parsed = urlsplit(raw)
    if parsed.scheme or parsed.netloc:
        return None
    path = unquote(parsed.path)
    if not path:
        return None
    # GitHub-style root relative references are repository-root relative.
    return (ROOT / path.lstrip("/")) if path.startswith("/") else (base / path)


class ResourceParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self.paths: list[tuple[str, str]] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        data = dict(attrs)
        if tag in ("img", "script", "link"):
            key = "href" if tag == "link" else "src"
            if data.get(key):
                self.paths.append((tag, data[key] or ""))


for name in REQUIRED:
    ensure_file(ROOT / name, "required artifact")

for svg in ROOT.rglob("*.svg"):
    try:
        tree = ET.parse(svg)
        if not tree.getroot().tag.endswith("svg"):
            errors.append(f"INVALID SVG ROOT: {svg.relative_to(ROOT)}")
    except ET.ParseError as exc:
        errors.append(f"INVALID SVG: {svg.relative_to(ROOT)}: {exc}")

for html in ROOT.rglob("*.html"):
    parser = ResourceParser()
    parser.feed(html.read_text(encoding="utf-8"))
    for tag, ref in parser.paths:
        path = local_path(ref, html.parent)
        if path is not None:
            ensure_file(path, f"{html.relative_to(ROOT)} {tag} reference {ref}")

for css in ROOT.rglob("*.css"):
    for ref in re.findall(r"url\(\s*['\"]?([^)'\"\s]+)", css.read_text(encoding="utf-8")):
        path = local_path(ref, css.parent)
        if path is not None:
            ensure_file(path, f"{css.relative_to(ROOT)} CSS url {ref}")

for markdown in (ROOT / "README.md", *ROOT.glob("docs/*.md"), ROOT / "CONTRIBUTING.md"):
    content = markdown.read_text(encoding="utf-8")
    for ref in re.findall(r"(?<!!)\[[^\]\n]+\]\(([^)]+)\)|!\[[^\]\n]*\]\(([^)]+)\)", content):
        raw = ref[0] or ref[1]
        if raw.startswith("<") and raw.endswith(">"):
            raw = raw[1:-1]
        path = local_path(raw, markdown.parent)
        if path is not None:
            ensure_file(path, f"{markdown.relative_to(ROOT)} Markdown link {raw}")

for qml in sorted((ROOT / "desktop").glob("*.qml")):
    content = qml.read_text(encoding="utf-8")
    for match in QML_PROPERTY.finditer(content):
        if match.group(1) in ITEM_PROPERTIES:
            line = content.count("\n", 0, match.start()) + 1
            errors.append(f"QML: {qml.relative_to(ROOT)}:{line}: property '{match.group(1)}' redeclares a property "
                          "of Item; Qt refuses to load the component. Pick another name.")

# Nothing private in the repository, from anyone: a collaborator's pull request runs this check too. Keys and
# tokens, personal email addresses, and files that hold a learner's data (the database, backups, viewing
# histories) are refused. Only files git tracks are read, so local data next to the checkout is not reported.
SECRET_PATTERNS = {
    "private key": re.compile(r"-----BEGIN [A-Z ]*PRIVATE KEY-----"),
    "GitHub token": re.compile(r"\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,})"),
    "AWS key": re.compile(r"\bAKIA[0-9A-Z]{16}\b"),
    "Google API key": re.compile(r"\bAIza[0-9A-Za-z_-]{35}\b"),
    "Slack token": re.compile(r"\bxox[abprs]-[A-Za-z0-9-]{10,}"),
    "API secret key": re.compile(r"\bsk-(?:ant-|proj-|live_)?[A-Za-z0-9_-]{32,}"),
    "Stripe key": re.compile(r"\b[rs]k_live_[A-Za-z0-9]{20,}"),
}
EMAIL = re.compile(r"[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}")
# Addresses that are fine in public: GitHub's private ones, and documentation placeholders.
EMAIL_ALLOWED = re.compile(r"@(?:users\.noreply\.github\.com|example\.(?:com|org|net))$|^noreply@", re.I)
PRIVATE_FILES = re.compile(
    r"(?:^|/)(?:\.env(?:\..+)?|[^/]+\.(?:sqlite3?|db|db-journal|db-wal|pem|key|p12|p8|pfx|jks|keystore"
    r"|mobileprovision|sqlite3?\.v\d+\.bak)|\.netrc|\.npmrc|learnquick-backup-[^/]*\.json|NetflixViewingHistory[^/]*\.csv"
    r"|ViewingActivity[^/]*\.csv)$|(?:^|/)netflix-report[^/]*/", re.I)
ALLOWED_PRIVATE_FILES = {".env.example"}
# Third-party copyright notices keep their authors' addresses, as their licenses require.
THIRD_PARTY_NOTICES = re.compile(r"^(?:THIRD_PARTY_NOTICES\.md|LICENSES/[^/]+\.txt)$")


def tracked_files() -> list[str]:
    import subprocess
    try:
        out = subprocess.run(["git", "-C", str(ROOT), "ls-files", "-z"], capture_output=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return []  # not a git checkout: nothing is about to be published from here
    return [name for name in out.decode("utf-8").split("\0") if name]


for name in tracked_files():
    if PRIVATE_FILES.search(name) and Path(name).name not in ALLOWED_PRIVATE_FILES:
        errors.append(f"PRIVATE FILE: {name} holds keys or a learner's data; remove it from git and keep it local.")
        continue
    path = ROOT / name
    try:
        if not path.is_file() or path.stat().st_size > 5_000_000:
            continue
        text = path.read_bytes().decode("utf-8")
    except (OSError, UnicodeDecodeError):
        continue  # binary files: images, icons, the voice model
    for line_number, line in enumerate(text.splitlines(), 1):
        for label, pattern in SECRET_PATTERNS.items():
            if pattern.search(line):
                errors.append(f"SECRET: {name}:{line_number}: looks like a {label}. Revoke it, then remove it.")
        for address in ([] if THIRD_PARTY_NOTICES.match(name) else EMAIL.findall(line)):
            if not EMAIL_ALLOWED.search(address):
                errors.append(f"EMAIL: {name}:{line_number}: {address} is a personal address. Use the GitHub "
                              "noreply address or a placeholder at example.com.")

if errors:
    for item in errors:
        print(item, file=sys.stderr)
    raise SystemExit(f"Validation failed: {len(errors)} error(s).")

print(f"Validation passed: {sum(1 for _ in ROOT.rglob('*.svg'))} SVGs and local repository references.")
