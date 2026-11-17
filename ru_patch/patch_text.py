with open("src/Text.cpp", "r", encoding="utf-8") as f:
    src = f.read()

old_case = """char Localization::toLower(char c) {
        if (c >= 'A' && c <= 'Z') {
                return (char)(c - 'A' + 'a');
        }
        return c;
}

char Localization::toUpper(char c) {
        if (c >= 'a' && c <= 'z') {
                return (char)(c - 'a' + 'A');
        }
        return c;
}"""

new_case = """char Localization::toLower(char c) {
        uint8_t uc = (uint8_t)c;
        if (uc >= 'A' && uc <= 'Z') return (char)(uc - 'A' + 'a');
        if (uc >= 0xC0 && uc <= 0xDF) return (char)(uc + 32);
        if (uc == 0xA8) return (char)0xB8;
        return c;
}

char Localization::toUpper(char c) {
        uint8_t uc = (uint8_t)c;
        if (uc >= 'a' && uc <= 'z') return (char)(uc - 'a' + 'A');
        if (uc >= 0xE0 && uc <= 0xFF) return (char)(uc - 32);
        if (uc == 0xB8) return (char)0xA8;
        return c;
}"""

assert old_case in src, "Не найдены toLower/toUpper в Text.cpp"
src = src.replace(old_case, new_case, 1)

SLOTS = tuple(i for i in range(96, 144) if i not in (99, 105))
CHARS = "БГДЁЖЗИЙЛПУФЦЧШЩЪЫЬЭЮЯбвгджзийклмнптфцчшщыьэюя"
LATIN_LOOKALIKE = {
    "А": "A", "В": "B", "Е": "E", "К": "K", "М": "M", "Н": "H",
    "О": "O", "Р": "P", "С": "C", "Т": "T", "Х": "X",
    "а": "a", "е": "e", "о": "o", "р": "p", "с": "c", "у": "y", "х": "x",
    "ё": "Ё", "ъ": "Ъ"
}

char_to_slot = dict(zip(CHARS, SLOTS))
cases = list()
for ch in "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя":
    b = next(iter(ch.encode("cp1251")))
    target = LATIN_LOOKALIKE.get(ch, ch)
    if target in char_to_slot:
        slot = char_to_slot.get(target)
    else:
        slot = ord(target) - ord("!")
    cases.append(f"        case 0x{b:02X}: *i = {slot}; *i2 = 0; return;")

ru_block = "    uint8_t chr = c & 0xff;\n    switch (chr) {\n" + "\n".join(cases) + "\n    }\n"
target_line = "    uint8_t chr = c & 0xff;"
assert target_line in src, "Не найдена getCharIndices в Text.cpp"
src = src.replace(target_line, ru_block, 1)

with open("src/Text.cpp", "w", encoding="utf-8") as f:
    f.write(src)
print("OK: src/Text.cpp пропатчен для кириллицы Windows-1251!")
