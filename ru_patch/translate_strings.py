import re, time, json, urllib.request, urllib.parse

def protect_tokens(text):
    clean = re.sub(r'(\w{2})-(\w{2,})', lambda m: m.group(1) + m.group(2) if m.group(2).islower() else m.group(0), text)
    tokens = list()
    def repl(m):
        tokens.append(m.group(0))
        return f" __T{len(tokens)-1}__ "
    masked = re.sub(r'(\|+|\\n|\^\d|%[sd0-9]|\\[A-O])', repl, clean)
    return masked, tokens

def restore_tokens(text, tokens):
    def repl(m):
        idx = int(m.group(1))
        return tokens.__getitem__(idx) if 0 <= idx < len(tokens) else m.group(0)
    res = re.sub(r'\s*__T(\d+)__\s*', repl, text)
    return res.strip()

def gtranslate_batch(lines):
    base = urllib.parse.urlunsplit(("https", "translate" + ".googleapis.com", "/translate_a/single", "client=gtx&sl=en&tl=ru&dt=t&q=", ""))
    joined = "\n@@@\n".join(lines)
    url = base + urllib.parse.quote(joined)
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req, timeout=20) as resp:
        data = json.loads(resp.read().decode("utf-8"))
    translated = "".join(seg.__getitem__(0) for seg in data.__getitem__(0) if seg.__getitem__(0))
    parts = re.split(r'\s*@@@\s*', translated)
    if len(parts) == len(lines):
        return parts
    out = list()
    for single in lines:
        u = base + urllib.parse.quote(single)
        r = urllib.request.Request(u, headers={"User-Agent": "Mozilla/5.0"})
        with urllib.request.urlopen(r, timeout=20) as rsp:
            d = json.loads(rsp.read().decode("utf-8"))
        out.append("".join(s.__getitem__(0) for s in d.__getitem__(0) if s.__getitem__(0)))
        time.sleep(0.05)
    return out

with open("strings_en.txt", "r", encoding="utf-8") as f:
    raw_lines = f.read().splitlines()

entries = list()
for idx, line in enumerate(raw_lines):
    if line.startswith("(") and len(line) >= 9 and line.__getitem__(7) == ")":
        prefix = line.__getitem__(slice(0, 9))
        body = line.__getitem__(slice(9, None))
        entries.append((idx, prefix, body))

print(f"Translating {len(entries)} lines to Russian...")
BATCH = 20
for start in range(0, len(entries), BATCH):
    chunk = entries.__getitem__(slice(start, start + BATCH))
    masked_list = list()
    tokens_list = list()
    for _, _, body in chunk:
        if not body.strip():
            masked_list.append("EMPTY")
            tokens_list.append(None)
        else:
            m, t = protect_tokens(body)
            masked_list.append(m)
            tokens_list.append(t)

    tr_list = gtranslate_batch(masked_list)
    for (line_idx, prefix, orig_body), tr_text, toks in zip(chunk, tr_list, tokens_list):
        if toks is None:
            final_text = orig_body
        else:
            final_text = restore_tokens(tr_text, toks)
        raw_lines.__setitem__(line_idx, prefix + final_text)

    if (start // BATCH) % 10 == 0:
        print(f"Progress: {min(start + BATCH, len(entries))}/{len(entries)}")
    time.sleep(0.15)

with open("strings_ru.txt", "w", encoding="utf-8") as f:
    f.write("\n".join(raw_lines) + "\n")
print("OK: перевод сохранён в ~/DoomIIRPG-RE/strings_ru.txt!")
