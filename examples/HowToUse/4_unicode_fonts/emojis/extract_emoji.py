"""

Copyright 2026 d4rkmen

https://github.com/d4rkmen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---------------------------------------------

Extract color emoji bitmaps from NotoColorEmoji-Regular.ttf as individual PNG files.

Output naming: u{CODEPOINT}.png  e.g. u1F450.png

Supports three color emoji formats:
  - CBDT/CBLC  (Google bitmap emoji)
  - sbix       (Apple bitmap emoji)
  - SVG        (Google/Adobe vector emoji — rendered via resvg)

Usage:
    python extract_emoji.py NotoColorEmoji-Regular.ttf
    python extract_emoji.py NotoColorEmoji-Regular.ttf --output emojis --resize 32
    python extract_emoji.py NotoColorEmoji-Regular.ttf --codepoint 1F450

Requirements:
    pip install fonttools Pillow resvg-python
"""

import argparse
import os
import re
import sys
from io import BytesIO

from fontTools.ttLib import TTFont
from PIL import Image

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
GLYPH_NAME_RE = re.compile(r"(?:u|uni)([0-9a-fA-F]+)")


def build_glyph_to_codepoint(ttfont):
    """Reverse-map glyph names to unicode codepoints via cmap."""
    cmap = ttfont.getBestCmap()
    glyph_to_cp = {}
    if cmap:
        for cp, glyph_name in cmap.items():
            glyph_to_cp[glyph_name] = cp
    return glyph_to_cp


def codepoint_from_name(name, glyph_to_cp):
    """Resolve a glyph name to its unicode codepoint."""
    if name in glyph_to_cp:
        return glyph_to_cp[name]
    m = GLYPH_NAME_RE.match(name)
    if m:
        return int(m.group(1), 16)
    return None


def format_filename(codepoint):
    return f"u{codepoint:04X}.png"


def _safe_char(cp):
    """Return the emoji character for display, or empty string if the console can't handle it."""
    if cp <= 0x20:
        return ""
    try:
        ch = chr(cp)
        ch.encode(sys.stdout.encoding or "utf-8")
        return ch
    except (UnicodeEncodeError, UnicodeDecodeError):
        return ""


def save_png(data, path, resize):
    """Save raw PNG data, optionally resizing."""
    if resize:
        img = Image.open(BytesIO(data))
        img = img.resize((resize, resize), Image.LANCZOS)
        img.save(path)
    else:
        with open(path, "wb") as f:
            f.write(data)


def extract_cbdt(ttfont, glyph_to_cp, output_dir, resize, only_cp):
    """Extract embedded PNGs from the CBDT table (bitmap emoji format)."""
    cbdt = ttfont.get("CBDT")
    if cbdt is None:
        return set()

    extracted = set()
    for strike_data in cbdt.strikeData:
        for glyph_name, bitmap in strike_data.items():
            cp = codepoint_from_name(glyph_name, glyph_to_cp)
            if cp is None or cp in extracted:
                continue
            if only_cp and cp != only_cp:
                continue

            offset = bitmap.data.find(PNG_SIGNATURE)
            if offset == -1:
                continue

            png_data = bitmap.data[offset:]
            filename = format_filename(cp)
            try:
                save_png(png_data, os.path.join(output_dir, filename), resize)
                extracted.add(cp)
                print(f"  {_safe_char(cp)} U+{cp:04X} -> {filename}")
            except Exception as e:
                print(f"  U+{cp:04X} error: {e}", file=sys.stderr)

    return extracted


def extract_sbix(ttfont, glyph_to_cp, output_dir, resize, only_cp):
    """Extract embedded PNGs from the sbix table (Apple bitmap emoji format)."""
    sbix = ttfont.get("sbix")
    if sbix is None:
        return set()

    best_strike = max(sbix.strikes.keys())
    extracted = set()

    for glyph in sbix.strikes[best_strike].glyphs.values():
        cp = codepoint_from_name(glyph.glyphName, glyph_to_cp)
        if cp is None or cp in extracted:
            continue
        if only_cp and cp != only_cp:
            continue
        if glyph.graphicType != "png ":
            continue

        filename = format_filename(cp)
        try:
            save_png(glyph.imageData, os.path.join(output_dir, filename), resize)
            extracted.add(cp)
            print(f"  {_safe_char(cp)} U+{cp:04X} -> {filename}")
        except Exception as e:
            print(f"  U+{cp:04X} error: {e}", file=sys.stderr)

    return extracted


def _build_svg_index(ttfont):
    """Build a mapping from glyph ID to SVG document entry."""
    svg_table = ttfont.get("SVG ")
    if svg_table is None:
        return {}
    index = {}
    for entry in svg_table.docList:
        for gid in range(entry.startGlyphID, entry.endGlyphID + 1):
            index[gid] = entry
    return index


def _extract_glyph_element(svg_data, gid):
    """
    Extract the <g id="glyphNNN">...</g> element from the SVG, handling nested <g> tags.
    Returns the matched string or empty string.
    """
    tag = f'<g id="glyph{gid}"'
    start = svg_data.find(tag)
    if start == -1:
        return ""

    depth = 0
    i = start
    while i < len(svg_data):
        if svg_data[i:i+2] == "<g":
            next_ch = svg_data[i+2:i+3]
            if next_ch in (" ", ">", "\t", "\n", "\r"):
                depth += 1
        elif svg_data[i:i+4] == "</g>":
            depth -= 1
            if depth == 0:
                return svg_data[start:i+4]
        i += 1

    return ""


def _render_svg_glyph(svg_data, gid, vb_x, vb_y, vb_w, vb_h, render_size):
    """
    Extract one glyph from a multi-glyph SVG document and render it to PNG bytes.
    """
    import resvg_python

    defs_match = re.search(r"(<defs>.*?</defs>)", svg_data, re.DOTALL)
    glyph_str = _extract_glyph_element(svg_data, gid)

    defs_str = defs_match.group(1) if defs_match else ""

    if not glyph_str:
        return None

    standalone_svg = (
        f'<svg xmlns="http://www.w3.org/2000/svg"'
        f' xmlns:xlink="http://www.w3.org/1999/xlink"'
        f' viewBox="{vb_x} {vb_y} {vb_w} {vb_h}"'
        f' width="{render_size}" height="{render_size}">'
        f'{defs_str}{glyph_str}'
        f'</svg>'
    )

    return bytes(resvg_python.svg_to_png(standalone_svg))


def extract_svg(ttfont, glyph_to_cp, output_dir, resize, only_cp):
    """Extract and render glyphs from the SVG table."""
    svg_table = ttfont.get("SVG ")
    if svg_table is None:
        return set()

    try:
        import resvg_python  # noqa: F401
    except ImportError:
        print("  WARNING: resvg-python not installed. Install with: pip install resvg-python", file=sys.stderr)
        print("  SVG emoji cannot be rendered without it.", file=sys.stderr)
        return set()

    glyph_order = ttfont.getGlyphOrder()
    hmtx = ttfont["hmtx"]
    os2 = ttfont["OS/2"]
    ascent = os2.sTypoAscender    # 950 for NotoColorEmoji
    descent = os2.sTypoDescender  # -250 for NotoColorEmoji
    total_h = ascent - descent

    cmap = ttfont.getBestCmap() or {}

    svg_index = _build_svg_index(ttfont)
    render_size = 128
    extracted = set()

    for cp, glyph_name in cmap.items():
        if only_cp and cp != only_cp:
            continue
        if cp in extracted:
            continue

        try:
            gid = glyph_order.index(glyph_name)
        except ValueError:
            continue

        entry = svg_index.get(gid)
        if entry is None:
            continue

        # Per-glyph advance width from hmtx; fall back to head.xMax
        aw = hmtx.metrics[glyph_name][0] if glyph_name in hmtx.metrics else total_h

        # Square viewBox that fits the full glyph without clipping
        dim = max(aw, total_h)
        vb_x = (aw - dim) // 2
        vb_y = -ascent - (dim - total_h) // 2
        vb_w = dim
        vb_h = dim

        try:
            png_data = _render_svg_glyph(entry.data, gid, vb_x, vb_y, vb_w, vb_h, render_size)
        except Exception as e:
            print(f"  U+{cp:04X} SVG render error: {e}", file=sys.stderr)
            continue

        if png_data is None:
            continue

        filename = format_filename(cp)
        try:
            save_png(png_data, os.path.join(output_dir, filename), resize)
            extracted.add(cp)
            print(f"  {_safe_char(cp)} U+{cp:04X} -> {filename}")
        except Exception as e:
            print(f"  U+{cp:04X} error: {e}", file=sys.stderr)

    return extracted


def main():
    parser = argparse.ArgumentParser(
        description="Extract color emoji from font files as individual PNGs"
    )
    parser.add_argument("font", help="Path to the .ttf font file")
    parser.add_argument("--output", default="png", help="Output directory (default: png)")
    parser.add_argument("--resize", type=int, default=None, help="Resize emoji to NxN pixels")
    parser.add_argument("--codepoint", type=str, default=None, help="Extract single codepoint, e.g. 1F450")
    args = parser.parse_args()

    if not os.path.isfile(args.font):
        print(f"Font not found: {args.font}", file=sys.stderr)
        sys.exit(1)

    os.makedirs(args.output, exist_ok=True)

    only_cp = int(args.codepoint, 16) if args.codepoint else None

    print(f"Loading font: {args.font}")
    ttfont = TTFont(args.font, fontNumber=0)
    glyph_to_cp = build_glyph_to_codepoint(ttfont)
    total_glyphs = len(glyph_to_cp)
    print(f"Found {total_glyphs} mapped glyphs in cmap")

    tables = sorted(ttfont.reader.tables.keys())
    print(f"Font tables: {', '.join(t.strip() for t in tables)}")

    extracted = set()

    if "CBDT" in ttfont:
        print("\nExtracting from CBDT (color bitmap) table...")
        extracted |= extract_cbdt(ttfont, glyph_to_cp, args.output, args.resize, only_cp)

    if "sbix" in ttfont:
        print("\nExtracting from sbix (Apple bitmap) table...")
        extracted |= extract_sbix(ttfont, glyph_to_cp, args.output, args.resize, only_cp)

    if "SVG " in ttfont:
        print("\nExtracting from SVG table...")
        extracted |= extract_svg(ttfont, glyph_to_cp, args.output, args.resize, only_cp)

    if not extracted:
        print("\nNo extractable emoji found.")
        if "COLR" in ttfont and "SVG " not in ttfont:
            print("This font uses COLR (vector layers) without SVG — not yet supported.")

    ttfont.close()

    print(f"\nDone. Extracted {len(extracted)} emoji to {os.path.abspath(args.output)}/")


if __name__ == "__main__":
    main()
