import re, time, json, urllib.request, urllib.parse

def gtrans(s):
    base = urllib.parse.urlunsplit(("https", "translate" + ".googleapis.com", "/translate_a/single", "client=gtx&sl=en&tl=ru&dt=t&q=", ""))
    url = base + urllib.parse.quote(s)
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req, timeout=5) as resp:
        data = json.loads(resp.read().decode("utf-8"))
    return "".join(seg.__getitem__(0) for seg in data.__getitem__(0) if seg.__getitem__(0))

with open("strings_ru.txt", "r", encoding="utf-8") as f:
    lines = f.read().splitlines()

fixed = list()
count = 0
for line in lines:
    if line.startswith("(") and len(line) >= 9 and line.__getitem__(7) == ")":
        prefix = line.__getitem__(slice(0, 9))
        body = line.__getitem__(slice(9, None))
        # Если в строке остались 3+ подряд английских слова
        if re.search(r'\b[A-Za-z]{2,}\s+[A-Za-z]{2,}\s+[A-Za-z]{2,}\b', body):
            clean = re.sub(r'(\w{2})-(\w{2,})', lambda m: m.group(1) + m.group(2), body)
            clean = clean.replace("\\n", " | ")
            try:
                tr = gtrans(clean).replace(" | ", "\\n")
                tr = tr.replace("«", '"').replace("»", '"').replace("—", "-")
                body = tr
                count += 1
                if count % 5 == 0: print(f"Допереведено строк: {count}...")
                time.sleep(0.08)
            except Exception:
                pass
        fixed.append(prefix + body)
    else:
        fixed.append(line)

with open("strings_ru.txt", "w", encoding="utf-8") as f:
    f.write("\n".join(fixed) + "\n")
print(f"OK: допереведено оставшихся английских строк: {count}")
