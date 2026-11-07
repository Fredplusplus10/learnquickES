"""Writes what espeak-ng says for every Spanish clause the app reads aloud.

The bundled Piper voice was trained on espeak-ng's phonemes (voice es-419). The app phonemizes with its
own rules instead of linking espeak-ng (GPL-3.0); tests/phonemizer_tests.cpp checks those rules against
this file, clause by clause.

    pip install espeakng-loader==0.2.4
    python desktop/tools/phoneme_reference.py > desktop/tests/data/phonemes_es419.json
"""
from __future__ import annotations

import ctypes
import json
import re
import sys
from pathlib import Path

import espeakng_loader

ROOT = Path(__file__).resolve().parents[1]
VOICE = "es-419"

# Course fields the app passes to the voice (SpeakButton, listening exercises, feedback).
SPOKEN_KEYS = {"es", "example", "phrases", "model", "audio", "answer", "answers", "lines", "sentence", "listeningText"}

# Words that exercise the rules beyond the course text: letters with several sounds, stress, diphthongs.
EXTRA = """
hola gente guerra guitarra pingüino vergüenza cigüeña llave yo hoy muy rey ley ayer calle ñandú chico
jamón gigante girasol queso quince cine zapato cielo examen México taxi exacto xilófono hielo hueso huevo
ahora prohibir búho carro perro radio honra alrededor Enrique Israel enredo mundo ángel anglosajón tango
cinco banco un gato en Barcelona con ganas un beso envío ambos mismo desde isla rasgo los dedos las vacas
la boca el dedo el gato algo verde caldo usted Madrid ciudad salud reloj país baúl río oído ciudadano
reír raíz maíz ruido cuidado fui huir oiga aire auto Europa reina boina aunque cuatro agua antiguo
rápidamente fácilmente café sofá papá también están comí árbol lápiz útil fútbol miércoles sábado
psicología kilo whisky web wifi
""".split()


def spoken_strings(node, key=""):
    if isinstance(node, dict):
        for k, v in node.items():
            yield from spoken_strings(v, k)
    elif isinstance(node, list):
        for v in node:
            yield from spoken_strings(v, key)
    elif isinstance(node, str) and key in SPOKEN_KEYS:
        yield node


def clauses(text: str):
    """Splits on the punctuation Piper turns into pauses; drops the marks the voice never reads."""
    text = text.replace("…", ",").replace("...", ",").replace("+", ",").replace("/", ",")
    text = re.sub(r"[()«»\"“”—–¿¡]", " ", text)
    for part in re.split(r"[.,;:?!]", text):
        part = " ".join(part.split())
        if part:
            yield part


class ESpeak:
    def __init__(self):
        self.lib = ctypes.cdll.LoadLibrary(espeakng_loader.get_library_path())
        self.lib.espeak_Initialize.restype = ctypes.c_int
        data = espeakng_loader.get_data_path().encode()
        # AUDIO_OUTPUT_SYNCHRONOUS: no sound device; the data path is the espeak-ng-data folder itself.
        if self.lib.espeak_Initialize(2, 0, data, 0) <= 0:
            raise SystemExit("espeak-ng did not start")
        self.lib.espeak_SetVoiceByName.argtypes = [ctypes.c_char_p]
        if self.lib.espeak_SetVoiceByName(VOICE.encode()) != 0:
            raise SystemExit(f"espeak-ng has no voice {VOICE}")
        self.lib.espeak_TextToPhonemes.restype = ctypes.c_char_p
        self.lib.espeak_TextToPhonemes.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.c_int, ctypes.c_int]
        self.lib.espeak_Info.restype = ctypes.c_char_p

    def version(self) -> str:
        return self.lib.espeak_Info(None).decode()

    def phonemes(self, clause: str) -> str:
        buffer = ctypes.create_string_buffer(clause.encode("utf-8"))
        pointer = ctypes.c_void_p(ctypes.addressof(buffer))
        out = []
        # espeakCHARS_UTF8 = 1, espeakPHONEMES_IPA = 2; one call per clause until the text is used up.
        while pointer.value:
            out.append(self.lib.espeak_TextToPhonemes(ctypes.byref(pointer), 1, 2).decode("utf-8"))
        return " ".join(p.strip() for p in out if p.strip())


def main() -> None:
    course = json.loads((ROOT / "content" / "course.json").read_text(encoding="utf-8"))
    texts = sorted({c for s in spoken_strings(course) for c in clauses(s)} | set(EXTRA))
    espeak = ESpeak()
    reference = {
        "voice": VOICE,
        "espeak": espeak.version(),
        "clauses": {text: espeak.phonemes(text) for text in texts},
    }
    json.dump(reference, sys.stdout, ensure_ascii=False, indent=1, sort_keys=True)
    sys.stdout.write("\n")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8", newline="\n")
    main()
