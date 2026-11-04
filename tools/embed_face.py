#!/usr/bin/env python3
"""Resize a helmet/face image to 320x240 and emit src/face_jpg.h."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "assets" / "ranger-helmet.png"
JPG = ROOT / "assets" / "face-320x240.jpg"
PNG = ROOT / "assets" / "face-320x240.png"
HEADER = ROOT / "src" / "face_jpg.h"
TW, TH = 320, 240


def fit(im: Image.Image) -> Image.Image:
    rgb = im.convert("RGB")
    pixels = rgb.load()
    minx, miny, maxx, maxy = rgb.width, rgb.height, 0, 0
    for y in range(rgb.height):
        for x in range(rgb.width):
            r, g, b = pixels[x, y]
            if r + g + b > 25:
                minx = min(minx, x)
                miny = min(miny, y)
                maxx = max(maxx, x)
                maxy = max(maxy, y)
    cropped = rgb.crop((minx, miny, maxx + 1, maxy + 1))
    scale = min(TW / cropped.width, TH / cropped.height)
    nw = int(round(cropped.width * scale))
    nh = int(round(cropped.height * scale))
    fitted = cropped.resize((nw, nh), Image.Resampling.LANCZOS)
    canvas = Image.new("RGB", (TW, TH), (12, 12, 14))
    canvas.paste(fitted, ((TW - nw) // 2, (TH - nh) // 2))
    return canvas


def write_header(data: bytes) -> None:
    lines = [
        "#pragma once",
        "#include <cstdint>",
        f"// Auto-generated 320x240 JPEG, {len(data)} bytes",
        f"constexpr unsigned face_jpg_len = {len(data)};",
        "constexpr uint8_t face_jpg[] = {",
    ]
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02x}" for b in data[i : i + 16])
        lines.append(f"  {chunk},")
    lines.append("};")
    HEADER.write_text("\n".join(lines) + "\n")


def main() -> None:
    canvas = fit(Image.open(SRC))
    PNG.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(PNG)
    canvas.save(JPG, quality=92, optimize=True)
    write_header(JPG.read_bytes())
    print(f"wrote {JPG} and {HEADER}")


if __name__ == "__main__":
    main()
