"use client";

import { useEffect, useMemo, useRef, useState } from "react";

import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { ScrollArea } from "@/components/ui/scroll-area";
import { SAMPLE_LINES, VOICES, type VoiceInfo } from "@/lib/voices";

const STORAGE_KEY = "stackchan-chosen-voice";

function localeLabel(locale: string) {
  return locale.replace("en-", "");
}

export function VoicePicker() {
  const [query, setQuery] = useState("");
  const [gender, setGender] = useState<"All" | "Male" | "Female">("All");
  const [text, setText] = useState(SAMPLE_LINES[0]);
  const [rate, setRate] = useState(0);
  const [pitch, setPitch] = useState(0);
  const [selected, setSelected] = useState("en-US-AndrewMultilingualNeural");
  const [playing, setPlaying] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const audioRef = useRef<HTMLAudioElement | null>(null);

  useEffect(() => {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (saved) setSelected(saved);
  }, []);

  const featured = VOICES.filter((v) => v.featured);
  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase();
    return VOICES.filter((v) => {
      if (gender !== "All" && v.gender !== gender) return false;
      if (!q) return true;
      return (
        v.name.toLowerCase().includes(q) ||
        v.id.toLowerCase().includes(q) ||
        v.locale.toLowerCase().includes(q) ||
        v.blurb.toLowerCase().includes(q)
      );
    });
  }, [query, gender]);

  async function play(voice: VoiceInfo) {
    setError(null);
    setPlaying(voice.id);
    try {
      const res = await fetch("/api/speak", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          voice: voice.id,
          text,
          rate,
          pitch,
        }),
      });
      if (!res.ok) {
        const payload = (await res.json().catch(() => null)) as {
          error?: string;
        } | null;
        throw new Error(payload?.error ?? `HTTP ${res.status}`);
      }
      const blob = await res.blob();
      const url = URL.createObjectURL(blob);
      audioRef.current?.pause();
      const audio = new Audio(url);
      audioRef.current = audio;
      audio.onended = () => {
        URL.revokeObjectURL(url);
        setPlaying(null);
      };
      audio.onerror = () => {
        URL.revokeObjectURL(url);
        setPlaying(null);
        setError("Could not play this clip.");
      };
      await audio.play();
    } catch (err) {
      setPlaying(null);
      setError(err instanceof Error ? err.message : "Playback failed");
    }
  }

  function choose(voice: VoiceInfo) {
    setSelected(voice.id);
    localStorage.setItem(STORAGE_KEY, voice.id);
  }

  const chosen = VOICES.find((v) => v.id === selected);

  return (
    <div className="mx-auto flex max-w-5xl flex-col gap-6 px-4 py-8 md:py-10">
      <header className="flex flex-col gap-3">
        <p className="text-sm font-medium tracking-wide text-red-400 uppercase">
          StackChan voice lab
        </p>
        <h1 className="text-3xl font-semibold tracking-tight text-zinc-50 md:text-4xl">
          Hear a voice, then pick one.
        </h1>
        <p className="max-w-2xl text-sm leading-6 text-zinc-400 md:text-base">
          These are licensed-style neural voices you can preview now and later
          run on your home PC for the robot. They are not a cartoon
          character&apos;s voice. Click Play on a card. Save the one that fits
          the helmet.
        </p>
      </header>

      <section className="rounded-2xl border border-zinc-800 bg-zinc-900/70 p-4 md:p-5">
        <div className="flex flex-col gap-3">
          <label className="text-xs font-medium tracking-wide text-zinc-400 uppercase">
            Line to speak
          </label>
          <Input
            value={text}
            onChange={(e) => setText(e.target.value)}
            maxLength={280}
            className="h-11 border-zinc-700 bg-zinc-950 text-zinc-100"
          />
          <div className="flex flex-wrap gap-2">
            {SAMPLE_LINES.map((line) => (
              <Button
                key={line}
                type="button"
                variant="outline"
                size="sm"
                className="h-auto max-w-full whitespace-normal border-zinc-700 py-1 text-left text-zinc-300"
                onClick={() => setText(line)}
              >
                {line}
              </Button>
            ))}
          </div>
          <div className="grid gap-4 pt-2 md:grid-cols-2">
            <label className="flex flex-col gap-2 text-sm text-zinc-300">
              Speed {rate > 0 ? `+${rate}` : rate}%
              <input
                type="range"
                min={-40}
                max={40}
                value={rate}
                onChange={(e) => setRate(Number(e.target.value))}
                className="w-full accent-red-500"
              />
            </label>
            <label className="flex flex-col gap-2 text-sm text-zinc-300">
              Pitch {pitch > 0 ? `+${pitch}` : pitch} Hz
              <input
                type="range"
                min={-12}
                max={12}
                value={pitch}
                onChange={(e) => setPitch(Number(e.target.value))}
                className="w-full accent-red-500"
              />
            </label>
          </div>
        </div>
      </section>

      {chosen ? (
        <div className="flex flex-wrap items-center gap-3 rounded-xl border border-red-900/60 bg-red-950/40 px-4 py-3 text-sm text-red-100">
          <span className="font-medium">Saved pick:</span>
          <span>
            {chosen.name} ({chosen.id})
          </span>
          <Button
            type="button"
            size="sm"
            onClick={() => play(chosen)}
            disabled={playing === chosen.id}
          >
            {playing === chosen.id ? "Playing…" : "Hear saved"}
          </Button>
        </div>
      ) : null}

      {error ? (
        <p className="rounded-lg border border-red-800 bg-red-950/50 px-3 py-2 text-sm text-red-200">
          {error}
        </p>
      ) : null}

      <section className="flex flex-col gap-3">
        <h2 className="text-lg font-semibold text-zinc-100">Start here</h2>
        <div className="grid gap-3 sm:grid-cols-2 lg:grid-cols-3">
          {featured.map((voice) => (
            <VoiceCard
              key={voice.id}
              voice={voice}
              selected={selected === voice.id}
              playing={playing === voice.id}
              onPlay={() => play(voice)}
              onChoose={() => choose(voice)}
            />
          ))}
        </div>
      </section>

      <section className="flex flex-col gap-3">
        <div className="flex flex-col gap-3 md:flex-row md:items-end md:justify-between">
          <h2 className="text-lg font-semibold text-zinc-100">All English voices</h2>
          <div className="flex flex-wrap gap-2">
            {(["All", "Male", "Female"] as const).map((g) => (
              <Button
                key={g}
                type="button"
                size="sm"
                variant={gender === g ? "default" : "outline"}
                className={gender === g ? "" : "border-zinc-700"}
                onClick={() => setGender(g)}
              >
                {g}
              </Button>
            ))}
            <Input
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              placeholder="Filter by name or country"
              className="h-8 w-52 border-zinc-700 bg-zinc-950"
            />
          </div>
        </div>
        <ScrollArea className="h-[min(28rem,60vh)] rounded-xl border border-zinc-800">
          <ul className="divide-y divide-zinc-800">
            {filtered.map((voice) => (
              <li
                key={voice.id}
                className="flex flex-col gap-2 px-3 py-3 sm:flex-row sm:items-center sm:justify-between"
              >
                <div className="min-w-0">
                  <div className="flex flex-wrap items-center gap-2">
                    <span className="font-medium text-zinc-100">{voice.name}</span>
                    <Badge variant="outline" className="border-zinc-700 text-zinc-400">
                      {localeLabel(voice.locale)}
                    </Badge>
                    <Badge variant="secondary">{voice.gender}</Badge>
                    {selected === voice.id ? (
                      <Badge className="bg-red-600 text-white">saved</Badge>
                    ) : null}
                  </div>
                  <p className="mt-1 text-sm text-zinc-400">{voice.blurb}</p>
                </div>
                <div className="flex shrink-0 gap-2">
                  <Button
                    type="button"
                    variant="outline"
                    size="sm"
                    className="border-zinc-700"
                    onClick={() => play(voice)}
                    disabled={playing === voice.id}
                  >
                    {playing === voice.id ? "Playing…" : "Play"}
                  </Button>
                  <Button type="button" size="sm" onClick={() => choose(voice)}>
                    Use this
                  </Button>
                </div>
              </li>
            ))}
          </ul>
        </ScrollArea>
      </section>
    </div>
  );
}

function VoiceCard({
  voice,
  selected,
  playing,
  onPlay,
  onChoose,
}: {
  voice: VoiceInfo;
  selected: boolean;
  playing: boolean;
  onPlay: () => void;
  onChoose: () => void;
}) {
  return (
    <article
      className={`flex flex-col gap-3 rounded-2xl border p-4 ${
        selected
          ? "border-red-500 bg-red-950/30"
          : "border-zinc-800 bg-zinc-900/60"
      }`}
    >
      <div className="flex items-start justify-between gap-2">
        <div>
          <h3 className="font-semibold text-zinc-50">{voice.name}</h3>
          <p className="text-xs text-zinc-500">
            {voice.locale} · {voice.gender}
          </p>
        </div>
        {selected ? <Badge className="bg-red-600 text-white">saved</Badge> : null}
      </div>
      <p className="text-sm leading-5 text-zinc-400">{voice.blurb}</p>
      <div className="mt-auto flex gap-2">
        <Button
          type="button"
          className="flex-1 bg-red-600 text-white hover:bg-red-500"
          onClick={onPlay}
          disabled={playing}
        >
          {playing ? "Playing…" : "Play"}
        </Button>
        <Button type="button" variant="outline" className="border-zinc-700" onClick={onChoose}>
          Use this
        </Button>
      </div>
    </article>
  );
}
