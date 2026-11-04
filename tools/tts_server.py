#!/usr/bin/env python3
"""Local WAV TTS for StackChan. Default voice is Andrew."""

from __future__ import annotations

import json
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import edge_tts
import asyncio

HOST = "0.0.0.0"
PORT = 43146
DEFAULT_VOICE = "en-US-AndrewMultilingualNeural"


def synth_wav(text: str, voice: str) -> bytes:
    with tempfile.TemporaryDirectory() as tmp:
        mp3 = Path(tmp) / "a.mp3"
        wav = Path(tmp) / "a.wav"

        async def run() -> None:
            comm = edge_tts.Communicate(text, voice)
            await comm.save(str(mp3))

        asyncio.run(run())
        subprocess.check_call(
            [
                "ffmpeg",
                "-y",
                "-i",
                str(mp3),
                "-ar",
                "16000",
                "-ac",
                "1",
                "-c:a",
                "pcm_s16le",
                str(wav),
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return wav.read_bytes()


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt: str, *args) -> None:
        print(self.address_string(), fmt % args)

    def _send(self, code: int, body: bytes, ctype: str) -> None:
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self) -> None:  # noqa: N802
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
        self.end_headers()

    def do_POST(self) -> None:  # noqa: N802
        if self.path.split("?", 1)[0] != "/speak":
            self._send(404, b"not found", "text/plain")
            return
        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)
        try:
            payload = json.loads(raw.decode("utf-8"))
            text = str(payload.get("text", "")).strip()
            voice = str(payload.get("voice") or DEFAULT_VOICE)
        except (json.JSONDecodeError, UnicodeDecodeError):
            self._send(400, b"bad json", "text/plain")
            return
        if not text or len(text) > 280:
            self._send(400, b"bad text", "text/plain")
            return
        try:
            wav = synth_wav(text, voice)
        except Exception as exc:  # noqa: BLE001
            print("synth failed", exc)
            self._send(502, str(exc).encode(), "text/plain")
            return
        self._send(200, wav, "audio/wav")


if __name__ == "__main__":
    print(f"TTS {DEFAULT_VOICE} on http://0.0.0.0:{PORT}/speak")
    ThreadingHTTPServer((HOST, PORT), Handler).serve_forever()
