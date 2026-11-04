import { execFile } from "node:child_process";
import { randomUUID } from "node:crypto";
import { readFile, unlink } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { promisify } from "node:util";

import { VOICE_IDS } from "@/lib/voices";

const execFileAsync = promisify(execFile);

function signedUnit(value: number, unit: string) {
  const n = Math.round(value);
  return `${n >= 0 ? "+" : ""}${n}${unit}`;
}

export async function POST(req: Request) {
  let body: { voice?: string; text?: string; rate?: number; pitch?: number };
  try {
    body = await req.json();
  } catch {
    return Response.json({ error: "Invalid JSON" }, { status: 400 });
  }

  const voice = body.voice ?? "";
  const text = (body.text ?? "").trim();
  if (!VOICE_IDS.has(voice)) {
    return Response.json({ error: "Unknown voice" }, { status: 400 });
  }
  if (!text || text.length > 280) {
    return Response.json({ error: "Text must be 1–280 characters" }, { status: 400 });
  }

  const rate = signedUnit(Math.max(-50, Math.min(50, body.rate ?? 0)), "%");
  const pitch = signedUnit(Math.max(-20, Math.min(20, body.pitch ?? 0)), "Hz");
  const out = join(tmpdir(), `ranger-voice-${randomUUID()}.mp3`);
  const script = join(process.cwd(), "scripts/speak.py");

  try {
    await execFileAsync(
      "python3",
      [
        script,
        "--voice",
        voice,
        "--text",
        text,
        "--rate",
        rate,
        "--pitch",
        pitch,
        "--out",
        out,
      ],
      { timeout: 30000 },
    );
    const mp3 = await readFile(out);
    return new Response(mp3, {
      headers: {
        "Content-Type": "audio/mpeg",
        "Cache-Control": "no-store",
      },
    });
  } catch (err) {
    const message = err instanceof Error ? err.message : "TTS failed";
    return Response.json({ error: message }, { status: 502 });
  } finally {
    await unlink(out).catch(() => undefined);
  }
}
