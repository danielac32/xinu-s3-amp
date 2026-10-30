#!/usr/bin/env python3
"""Build and validate the static Xinu web package.

Sources:
    docs/MANUAL-KERNEL.md    -> web/manual.html
    docs/MANUAL-LOADER.md    -> web/loader.html
    docs/MANUAL-AMP.md       -> web/amp.html
    docs/MANUAL-NATIVES.md   -> web/natives.html
    docs/INVENTARIO-AMP.md   -> web/inventory.html

The hand-written dashboard is web/index.html.  The build validates its local
links, copies the package to web/dist/ and prints the resulting sizes.

Usage:
    python3 web/build_web.py            # generate + validate + dist
    python3 web/build_web.py --check    # validate only
    python3 web/build_web.py --docs     # generate manuals only
"""
from __future__ import annotations

import argparse
import html
import re
import shutil
import subprocess
import sys
from pathlib import Path

WEB = Path(__file__).resolve().parent
ROOT = WEB.parent
DOCS = ROOT / "docs"
DIST = WEB / "dist"

sys.dont_write_bytecode = True
sys.path.insert(0, str(WEB))
from md2html import PAGE, convert  # noqa: E402

PAGES = [
    ("MANUAL-KERNEL.md", "manual.html", "Manual del kernel Xinu"),
    ("MANUAL-LOADER.md", "loader.html", "Manual del Xinu Loader"),
    ("MANUAL-AMP.md", "amp.html", "Manual del kernel AMP"),
    ("MANUAL-NATIVES.md", "natives.html", "Librerías Java y natives"),
    ("INVENTARIO-AMP.md", "inventory.html", "Inventario del árbol"),
]

LINK_MAP = {
    "MANUAL-KERNEL.md": "manual.html",
    "MANUAL-LOADER.md": "loader.html",
    "MANUAL-AMP.md": "amp.html",
    "MANUAL-NATIVES.md": "natives.html",
    "INVENTARIO-AMP.md": "inventory.html",
}


def rewrite_links(text: str) -> str:
    """Turn Markdown cross-references into HTML links."""
    def repl(m: re.Match[str]) -> str:
        href = m.group(1)
        base = href.split("#", 1)[0]
        if base in LINK_MAP:
            return 'href="%s%s"' % (LINK_MAP[base], href[len(base):])
        if base.endswith(".md"):
            return 'href="%s.html%s"' % (base[:-3], href[len(base):])
        return m.group(0)

    text = re.sub(r'href="([^"]+)"', repl, text)
    labels = {
        ">MANUAL-KERNEL.md</a>": ">Manual del kernel</a>",
        ">MANUAL-LOADER.md</a>": ">Manual del loader</a>",
        ">MANUAL-AMP.md</a>": ">Manual AMP</a>",
        ">MANUAL-NATIVES.md</a>": ">Manual de natives</a>",
        ">INVENTARIO-AMP.md</a>": ">Inventario del árbol</a>",
    }
    for old_label, new_label in labels.items():
        text = text.replace(old_label, new_label)
    return text


def render(src: Path, dst: Path, title: str) -> None:
    md = src.read_text(encoding="utf-8")
    body = rewrite_links(convert(md))
    page = PAGE.replace("@@BODY@@", body)
    page = page.replace(
        "<title>Manual del kernel Xinu · ESP32-S3</title>",
        "<title>%s · Xinu ESP32-S3</title>" % html.escape(title),
    )
    dst.write_text(page, encoding="utf-8")
    print("  %-22s %7d bytes" % (dst.name, len(page.encode("utf-8"))))


def generate_docs() -> None:
    print("Generando manuales:")
    for src_name, dst_name, title in PAGES:
        src = DOCS / src_name
        if not src.exists():
            raise SystemExit("falta %s" % src)
        render(src, WEB / dst_name, title)


def local_links(path: Path) -> list[tuple[str, str]]:
    text = path.read_text(encoding="utf-8")
    links = []
    for m in re.finditer(r'(?:href|src)="([^"]+)"', text):
        url = m.group(1)
        if not url or url.startswith(("#", "http://", "https://", "mailto:", "javascript:", "data:")):
            continue
        # Ignore runtime URLs built inside JavaScript strings and API calls.
        if "?" in url or any(ch in url for ch in "'\"{}$+`"):
            continue
        links.append((url, m.group(0)))
    return links


def validate() -> None:
    errors = []
    index = WEB / "index.html"
    if not index.exists():
        raise SystemExit("falta web/index.html")

    required = [
        "loader.html", "amp.html", "natives.html", "manual.html", "inventory.html",
    ]
    for name in required:
        if not (WEB / name).exists():
            errors.append("falta la página %s" % name)
        if name not in index.read_text(encoding="utf-8"):
            errors.append("index.html no enlaza %s" % name)

    for page in [index, WEB / "docs.html"] + [WEB / p[1] for p in PAGES]:
        if not page.exists():
            continue
        for url, raw in local_links(page):
            target = url.split("#", 1)[0]
            if not target:
                continue
            candidate = (page.parent / target).resolve()
            if not candidate.exists():
                errors.append("%s: enlace roto %s (%s)" % (page.name, url, raw))

    if errors:
        print("Validación FALLIDA:")
        for e in errors:
            print("  - %s" % e)
        raise SystemExit(1)
    print("Validación OK: %d páginas, enlaces locales correctos." % (len(PAGES) + 2))


def build_dist() -> None:
    if DIST.exists():
        shutil.rmtree(DIST)
    DIST.mkdir(parents=True)
    names = ["index.html", "docs.html"] + [p[1] for p in PAGES]
    for name in names:
        src = WEB / name
        if src.exists():
            shutil.copy2(src, DIST / name)
    print("Paquete web: %s" % DIST)
    for p in sorted(DIST.iterdir()):
        print("  %-22s %7d bytes" % (p.name, p.stat().st_size))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="solo validar")
    ap.add_argument("--docs", action="store_true", help="solo generar manuales")
    ap.add_argument("--no-dist", action="store_true", help="no crear web/dist")
    ap.add_argument("--skip-inventory", action="store_true", help="no regenerar inventario")
    args = ap.parse_args()

    if not args.check and not args.skip_inventory:
        inv = ROOT / "tools" / "gen_inventory.py"
        if inv.exists():
            subprocess.run([sys.executable, str(inv)], check=True)

    if not args.check:
        generate_docs()

    if not args.docs:
        validate()

    if not args.check and not args.docs and not args.no_dist:
        build_dist()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
