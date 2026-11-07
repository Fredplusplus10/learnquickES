"""Prototype of the es-419 phonemizer that the C++ port (lq_core) will follow.

Matches espeak-ng on 536 of 539 reference clauses (99.4%; 99.9% of phonemes):
    python desktop/tools/phonemizer_prototype.py desktop/tests/data/phonemes_es419.json
"""
import json
import sys
import unicodedata

sys.stdout.reconfigure(encoding='utf-8')

UNSTRESSED = {'a', 'al', 'con', 'de', 'del', 'el', 'en', 'la', 'las', 'le', 'les', 'lo', 'los', 'me', 'mi', 'mis',
              'nos', 'os', 'por', 'que', 'se', 'si', 'sin', 'su', 'sus', 'te', 'tu', 'tus', 'y', 'e', 'o', 'u', 'ni'}
SECONDARY = {'aunque', 'cuando', 'desde', 'para', 'porque', 'como', 'donde', 'pero', 'mientras'}
VOWELS = 'aeiou'
ACCENTED = {'á': 'a', 'é': 'e', 'í': 'i', 'ó': 'o', 'ú': 'u'}
CLUSTER_SECOND = {'l', 'ɾ'}
CLUSTER_FIRST = {'p', 'B', 'f', 'k', 'G', 't', 'D'}

UNITS = ['', 'uno', 'dos', 'tres', 'cuatro', 'cinco', 'seis', 'siete', 'ocho', 'nueve', 'diez', 'once', 'doce',
         'trece', 'catorce', 'quince', 'dieciséis', 'diecisiete', 'dieciocho', 'diecinueve', 'veinte', 'veintiuno',
         'veintidós', 'veintitrés', 'veinticuatro', 'veinticinco', 'veintiséis', 'veintisiete', 'veintiocho',
         'veintinueve']
TENS = ['', '', '', 'treinta', 'cuarenta', 'cincuenta', 'sesenta', 'setenta', 'ochenta', 'noventa']
HUNDREDS = ['', 'ciento', 'doscientos', 'trescientos', 'cuatrocientos', 'quinientos', 'seiscientos', 'setecientos',
            'ochocientos', 'novecientos']


def number_words(n):
    if n == 0:
        return ['cero']
    out = []
    if n >= 1000:
        k = n // 1000
        out += (['mil'] if k == 1 else number_words(k) + ['mil'])
        n %= 1000
    if n >= 100:
        out.append('cien' if n == 100 else HUNDREDS[n // 100])
        n %= 100
    if n >= 30:
        out.append(TENS[n // 10])
        if n % 10:
            out += ['y', UNITS[n % 10]]
    elif n:
        out.append(UNITS[n])
    return out


class Seg:
    """One sound before context rules: kind 'V' (vowel) or 'C' (consonant)."""
    def __init__(self, ph, kind, accent=False, firm=False):
        self.ph, self.kind, self.accent, self.firm = ph, kind, accent, firm
        self.syl = -1
        self.glide = False
        self.keep = False  # a weak vowel that stays a vowel inside its diphthong


def segments(word):
    w = word
    out = []
    i = 0
    n = len(w)

    def at(j):
        return w[j] if 0 <= j < n else ''
    while i < n:
        c = w[i]
        nxt = at(i + 1)
        if c in VOWELS or c in ACCENTED:
            # After a trilled r, or an l / r that starts the word or follows a consonant, a weak vowel
            # keeps its vowel sound: "cliente", "luego", "ruido", "barrio" (but "familia", "historia").
            liquid = out[-1] if out else None
            keep = False
            if c in 'iu' and liquid is not None and liquid.ph in ('l', 'r', 'ɾ'):
                before = out[-2] if len(out) > 1 else None
                keep = liquid.ph == 'r' or before is None or before.kind == 'C'
            seg = Seg(ACCENTED.get(c, c), 'V', accent=c in ACCENTED)
            seg.keep = keep
            out.append(seg)
        elif c == 'ü':
            out.append(Seg('u', 'V', firm=True))
        elif c == 'y':
            # A vowel at the end of a word ("hoy", "y"), a consonant before a vowel ("yo", "ayer").
            if nxt and (nxt in VOWELS or nxt in ACCENTED):
                out.append(Seg('ʝ', 'C'))
            elif out and out[-1].kind == 'V' and out[-1].ph == 'u' and not nxt:
                out.append(Seg('j', 'C'))  # "muy"
            else:
                out.append(Seg('i', 'V'))
        elif c in 'bv':
            out.append(Seg('B', 'C'))
        elif c == 'd':
            out.append(Seg('D', 'C'))
        elif c == 'g':
            if nxt in ('e', 'i', 'é', 'í'):
                out.append(Seg('x', 'C'))
            elif nxt == 'u' and at(i + 2) in ('e', 'i', 'é', 'í'):
                out.append(Seg('G', 'C'))
                i += 1
            else:
                out.append(Seg('G', 'C'))
        elif c == 'j':
            out.append(Seg('x', 'C'))
        elif c == 'c':
            if nxt == 'h':
                out.append(Seg('tʃ', 'C'))
                i += 1
            elif nxt in ('e', 'i', 'é', 'í'):
                if not (out and out[-1].ph == 's'):
                    out.append(Seg('s', 'C'))
            else:
                out.append(Seg('k', 'C'))
        elif c == 'q':
            out.append(Seg('k', 'C'))
            if nxt == 'u':
                i += 1
        elif c == 'z':
            out.append(Seg('s', 'C'))
        elif c == 'l':
            if nxt == 'l':
                if out:
                    out.append(Seg('j', 'C'))
                    out.append(Seg('j', 'C'))
                else:
                    out.append(Seg('ʝ', 'C'))
                i += 1
            else:
                out.append(Seg('l', 'C'))
        elif c == 'r':
            if nxt == 'r':
                out.append(Seg('r', 'C'))
                i += 1
            elif i == 0 or (out and out[-1].ph in ('n', 'l', 's')):
                out.append(Seg('r', 'C'))
            else:
                out.append(Seg('ɾ', 'C'))
        elif c == 'ñ':
            out.append(Seg('ɲ', 'C'))
        elif c == 'h':
            out.append(Seg('', 'H'))  # silent, but it keeps vowels apart: "prohibir", "Ohio"
        elif c == 'x':
            if i > 0:
                out.append(Seg('k', 'C'))
            out.append(Seg('s', 'C'))
        elif c == 'k':
            out.append(Seg('k', 'C'))
        elif c == 'w':
            out.append(Seg('u', 'V'))
        elif c == 'p' and i == 0 and nxt == 's':
            pass  # "psicología"
        elif c in 'fmnpt':
            out.append(Seg(c, 'C'))
        elif c == 's':
            if not (out and out[-1].ph == 's'):
                out.append(Seg('s', 'C'))
        i += 1
    return out


def weak(s):
    return s.kind == 'V' and s.ph in 'iu' and not s.accent and not s.firm


def syllabify(segs):
    """Assigns each segment a syllable index; returns the nucleus segment index of each syllable."""
    # Group vowels into nuclei.
    groups = []  # list of lists of segment indices (vowel runs that form one syllable)
    i = 0
    while i < len(segs):
        if segs[i].kind != 'V':
            i += 1
            continue
        run = [i]
        j = i + 1
        while j < len(segs) and segs[j].kind == 'V':
            run.append(j)
            j += 1
        # Split the run into syllables: two strong vowels or an accented weak vowel break it.
        cur = [run[0]]
        for k in run[1:]:
            prev = segs[cur[-1]]
            s = segs[k]
            together = (weak(prev) or weak(s)) and (weak(s) or all(weak(segs[x]) for x in cur)) \
                and not prev.firm and not s.firm
            if together and len(cur) < 3:
                cur.append(k)
            else:
                groups.append(cur)
                cur = [k]
        groups.append(cur)
        i = j
    nuclei = []
    for g in groups:
        # The nucleus is the strong or accented vowel; with two weak vowels, the second.
        strong = [k for k in g if not weak(segs[k])]
        nucleus = strong[0] if strong else g[-1]
        nuclei.append(nucleus)
        for k in g:
            segs[k].syl = len(nuclei) - 1
            segs[k].glide = k != nucleus
    # Consonants between groups.
    for gi in range(len(groups)):
        start = groups[gi][-1] + 1
        end = groups[gi + 1][0] if gi + 1 < len(groups) else len(segs)
        cons = list(range(start, end))
        if gi + 1 == len(groups):
            for k in cons:
                segs[k].syl = gi
            continue
        split = len(cons)  # how many go to the current syllable
        if len(cons) == 1:
            split = 0
        elif len(cons) >= 2:
            a, b = segs[cons[-2]].ph, segs[cons[-1]].ph
            if a in CLUSTER_FIRST and b in CLUSTER_SECOND and not (a == 'D' and b == 'l'):
                split = len(cons) - 2
            else:
                split = len(cons) - 1
        for idx, k in enumerate(cons):
            segs[k].syl = gi if idx < split else gi + 1
    for k in range(groups[0][0] if groups else len(segs)):
        segs[k].syl = 0
    return nuclei


def primary_stress(segs, nuclei, word):
    count = len(nuclei)
    for k, s in enumerate(segs):
        if s.accent:
            return s.syl
    if count <= 1:
        return 0
    last = word[-1]
    if last in 'aeiouns':
        return count - 2
    return count - 1


def word_phonemes(word, stress_mode):
    """stress_mode: 'primary', 'secondary' (reduced), or 'none'."""
    if word.endswith('mente') and len(word) > 7 and stress_mode == 'primary':
        return word_phonemes(word[:-5], 'primary') + word_phonemes('mente', 'primary', )
    segs = segments(word)
    if not segs:
        return []
    nuclei = syllabify(segs)
    if not nuclei:
        return [(s.ph, s) for s in segs]
    p = primary_stress(segs, nuclei, word)
    marks = {}
    if stress_mode == 'primary':
        marks[p] = 'ˈ'
        for k in range(0, p - 1, 2):
            marks[k] = 'ˌ'
    elif stress_mode == 'secondary':
        marks[p] = 'ˌ'
    out = []
    for k, s in enumerate(segs):
        if s.kind == 'V' and s.syl in marks and nuclei[s.syl] == k:
            out.append((marks[s.syl], None))
        ph = s.ph
        if s.kind == 'V' and s.glide and not s.keep:
            before_nucleus = k < nuclei[s.syl]
            if before_nucleus:
                ph = {'i': 'j', 'u': 'w'}[ph]
            else:
                ph = {'i': 'ɪ', 'u': 'ʊ'}[ph]
                if ph == 'ʊ' and k > 0 and segs[k - 1].ph == 'o':
                    ph = 'w'
        if ph == 'e' and s.kind == 'V' and not s.glide and marks.get(s.syl) == 'ˈ' and word != 'mente':
            rest = segs[k + 1:k + 3]
            if len(rest) == 2 and rest[0].ph == 'n' and rest[1].kind == 'C' and rest[1].syl != s.syl:
                ph = 'ɛ'
        out.append((ph, s))
    return out


def realize(words_out):
    """Applies context rules across the clause; words_out is a list of lists of (ph, seg)."""
    flat = []
    for wi, w in enumerate(words_out):
        for ph, seg in w:
            flat.append([ph, wi])
    res = []
    prev = None  # previous real phoneme (skipping stress marks)
    for idx, (ph, wi) in enumerate(flat):
        if ph in ('ˈ', 'ˌ', ''):
            if ph:
                res.append((ph, wi))
            continue
        following = [f for f in flat[idx + 1:] if f[0] not in ('ˈ', 'ˌ', '')]
        nxt = following[0][0] if following else None
        same_word = bool(following) and following[0][1] == wi
        word_final = not following or following[0][1] != wi
        if ph == 'B':
            ph = 'b' if prev is None or prev in ('m', 'n') or word_final else 'β'
        elif ph == 'D':
            ph = 'd' if prev is None or prev in ('n', 'm') or word_final else 'ð'
        elif ph == 'G':
            ph = 'ɡ' if prev is None or prev in ('n', 'ŋ') or nxt == 'n' or word_final else 'ɣ'
        elif ph == 'n':
            if nxt in ('B', 'p') or (nxt == 'f' and not same_word):
                ph = 'm'
            elif nxt in ('G', 'x') and same_word:
                ph = 'ŋ'
        res.append((ph, wi))
        prev = ph
    words = {}
    for ph, wi in res:
        words.setdefault(wi, []).append(ph)
    return ' '.join(''.join(words[k]) for k in sorted(words))


def tokens(text):
    text = unicodedata.normalize('NFC', text.lower())
    out = []
    import re
    for raw in text.split():
        for word in re.findall(r'\d+|[^\W\d_]+', raw):
            if word.isdigit():
                out += number_words(int(word))
            else:
                out.append(word)
    return out


EXCEPTIONS = {'méxico': 'mˈɛxiko', 'mexicano': 'mˌexikˈano', 'mexicana': 'mˌexikˈana', 'ser': 'sˈer'}
LETTERS = {'b': 'be', 'c': 'ce', 'd': 'de', 'f': 'efe', 'g': 'ge', 'h': 'hache', 'j': 'jota', 'k': 'ka',
           'l': 'ele', 'm': 'eme', 'n': 'ene', 'ñ': 'eñe', 'p': 'pe', 'q': 'cu', 'r': 'erre', 's': 'ese',
           't': 'te', 'v': 'be', 'w': 'doble be', 'x': 'equis', 'z': 'zeta'}


def phonemize(clause):
    words = []
    for w in tokens(clause):
        if not any(ch in VOWELS or ch in ACCENTED or ch == 'y' for ch in w):
            for ch in w:
                words += LETTERS.get(ch, '').split()
        else:
            words.append(w)
    outs = []
    for i, w in enumerate(words):
        if w in EXCEPTIONS:
            outs.append([(ch, None) for ch in EXCEPTIONS[w]])
            continue
        last = i == len(words) - 1
        if w in UNSTRESSED and not last:
            mode = 'none'
        elif w in SECONDARY and not last:
            mode = 'secondary'
        else:
            mode = 'primary'
        outs.append(word_phonemes(w, mode))
    return realize(outs)


if __name__ == '__main__':
    ref = json.load(open(sys.argv[1], encoding='utf-8'))['clauses']
    good = 0
    total_ph = 0
    errors = 0
    bad = []
    import difflib
    for text, want in ref.items():
        got = phonemize(text)
        if got == want:
            good += 1
        else:
            bad.append((text, want, got))
        sm = difflib.SequenceMatcher(a=want, b=got)
        total_ph += len(want)
        errors += sum(max(i2 - i1, j2 - j1) for op, i1, i2, j1, j2 in sm.get_opcodes() if op != 'equal')
    print(f'exact clauses: {good}/{len(ref)} = {good / len(ref):.1%}; phoneme accuracy {1 - errors / total_ph:.2%}')
    for text, want, got in bad[:int(sys.argv[2]) if len(sys.argv) > 2 else 40]:
        print(f'  {text}\n     want {want}\n     got  {got}')
