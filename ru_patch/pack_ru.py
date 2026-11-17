import sys, zipfile, struct

orig_ipa = sys.argv[1] if len(sys.argv) > 1 else "Doom 2 RPG.ipa"
out_ipa = sys.argv[2] if len(sys.argv) > 2 else "opk_tmp/Doom 2 RPG.ipa"
fonts = ("Font.bmp", "Font_16p_Dark.bmp", "Font_16p_Light.bmp", "Font_18p_Light.bmp", "WarFont.bmp")

def clean_ru(s):
    s = s.replace("\\n", "\n")
    s = s.replace("«", '"').replace("»", '"').replace("“", '"').replace("”", '"')
    s = s.replace("—", "-").replace("–", "-").replace("…", "...")
    return s.encode("cp1251", errors="replace")

banks_ru = dict((b, list()) for b in range(15))
with open("strings_ru.txt", "r", encoding="utf-8") as f:
    for line in f:
        line = line.rstrip("\r\n")
        if line.startswith("(") and len(line) >= 9 and line[7] == ")":
            b = int(line[1:3])
            body = line[9:]
            banks_ru[b].append(clean_ru(body))

with zipfile.ZipFile(orig_ipa, "r") as zin:
    idx_data = zin.read("Payload/Doom2rpg.app/Packages/strings.idx")
    orig_count = next(iter(struct.unpack_from("<h", idx_data, 0)))

    raw_records = []
    pos = 2
    bank_cursor = 0
    while pos + 5 <= len(idx_data):
        c_id, off = struct.unpack_from("<Bi", idx_data, pos)
        pos += 5
        if c_id != 0xFF:
            raw_records.append((c_id, off, bank_cursor))
            bank_cursor += 1
        else:
            raw_records.append((0xFF, off, -1))

    new_chunks = {0: bytearray(), 1: bytearray(), 2: bytearray()}
    new_idx = bytearray()
    new_idx.extend(struct.pack("<h", orig_count))

    last_chunk = 0
    for c_id, orig_off, b_idx in raw_records:
        if c_id != 0xFF:
            last_chunk = c_id
            cur_off = len(new_chunks[c_id])
            new_idx.extend(struct.pack("<Bi", c_id, cur_off))
            for item in banks_ru[b_idx]:
                new_chunks[c_id].extend(item + b"\x00")
        else:
            end_off = len(new_chunks[last_chunk])
            new_idx.extend(struct.pack("<Bi", 0xFF, end_off))

    replacements = {
        "Payload/Doom2rpg.app/Packages/strings.idx": bytes(new_idx),
        "Payload/Doom2rpg.app/Packages/strings00.bin": bytes(new_chunks[0]),
        "Payload/Doom2rpg.app/Packages/strings01.bin": bytes(new_chunks[1]),
        "Payload/Doom2rpg.app/Packages/strings02.bin": bytes(new_chunks[2]),
    }
    for fn in fonts:
        with open("ru_patch/" + fn, "rb") as ff:
            replacements["Payload/Doom2rpg.app/Packages/" + fn] = ff.read()

    with zipfile.ZipFile(out_ipa, "w", compression=zipfile.ZIP_DEFLATED) as zout:
        for info in zin.infolist():
            if info.filename in replacements:
                zout.writestr(info.filename, replacements[info.filename])
            else:
                zout.writestr(info, zin.read(info.filename))

print("OK: Doom 2 RPG.ipa patched with Russian localization and fonts!")
