#!/usr/bin/env python3
"""Synthesize a short MP3 with edge-tts."""

from __future__ import annotations

import argparse
import asyncio

import edge_tts


async def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--voice", required=True)
    parser.add_argument("--text", required=True)
    parser.add_argument("--rate", default="+0%")
    parser.add_argument("--pitch", default="+0Hz")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    communicate = edge_tts.Communicate(
        args.text, args.voice, rate=args.rate, pitch=args.pitch
    )
    await communicate.save(args.out)


if __name__ == "__main__":
    asyncio.run(main())
