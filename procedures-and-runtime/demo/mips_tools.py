#!/usr/bin/env python3
"""Assemble the CS241 MIPS dialect and link MERL libraries at address zero."""
import re
import struct
import sys
from pathlib import Path


def number(value):
    return int(value, 16 if value.lower().startswith('0x') else 10)


def register(value):
    if not re.fullmatch(r'\$\d+', value):
        raise ValueError(f'invalid register: {value}')
    result = int(value[1:])
    if not 0 <= result <= 31:
        raise ValueError(f'register out of range: {value}')
    return result


def assemble(path):
    labels, imports, exports, instructions = {}, set(), set(), []
    for line in Path(path).read_text().splitlines():
        line = line.split(';')[0].strip()
        while match := re.match(r'^([A-Za-z][A-Za-z0-9]*):', line):
            label = match[1]
            if label in labels:
                raise ValueError(f'duplicate label: {label}')
            labels[label] = 12 + 4 * len(instructions)
            line = line[match.end():].strip()
        if not line:
            continue
        parts = line.split()
        if parts[0] in ('.import', '.export'):
            if len(parts) != 2:
                raise ValueError(f'invalid directive: {line}')
            (imports if parts[0] == '.import' else exports).add(parts[1])
        else:
            instructions.append(line)
    words, relocations, references = [], [], []
    for index, line in enumerate(instructions):
        parts = re.split(r'[\s,()]+', line.strip())
        parts = [part for part in parts if part]
        op, args = parts[0], parts[1:]
        address = 12 + 4 * index
        if op == '.word' and len(args) == 1:
            value = args[0]
            if value in labels:
                word = labels[value]
                relocations.append(address)
            elif value in imports:
                word = 0
                references.append((address, value))
            else:
                word = number(value)
                if not -(1 << 31) <= word < (1 << 32):
                    raise ValueError(f'word out of range: {value}')
        elif op in ('add', 'sub', 'slt', 'sltu') and len(args) == 3:
            rd, rs, rt = map(register, args)
            word = rs << 21 | rt << 16 | rd << 11 | {'add': 32, 'sub': 34, 'slt': 42, 'sltu': 43}[op]
        elif op in ('mult', 'multu', 'div', 'divu') and len(args) == 2:
            rs, rt = map(register, args)
            word = rs << 21 | rt << 16 | {'mult': 24, 'multu': 25, 'div': 26, 'divu': 27}[op]
        elif op in ('lis', 'mfhi', 'mflo') and len(args) == 1:
            word = register(args[0]) << 11 | {'lis': 20, 'mfhi': 16, 'mflo': 18}[op]
        elif op in ('jr', 'jalr') and len(args) == 1:
            word = register(args[0]) << 21 | (8 if op == 'jr' else 9)
        elif op in ('beq', 'bne', 'lw', 'sw') and len(args) == 3:
            if op in ('lw', 'sw'):
                rt, rs = register(args[0]), register(args[2])
                immediate = number(args[1])
            else:
                rs, rt = register(args[0]), register(args[1])
                immediate = (labels[args[2]] - address - 4) // 4 if args[2] in labels else number(args[2])
            if not -32768 <= immediate <= (65535 if args[1 if op in ('lw', 'sw') else 2].startswith('0x') else 32767):
                raise ValueError(f'immediate out of range: {line}')
            word = {'beq': 4, 'bne': 5, 'lw': 35, 'sw': 43}[op] << 26 | rs << 21 | rt << 16 | (immediate & 65535)
        else:
            raise ValueError(f'unsupported instruction: {line}')
        words.append(word & 0xffffffff)
    if imports & labels.keys():
        raise ValueError('import conflicts with local label')
    return words, relocations, {name: labels[name] for name in exports}, references


def read_merl(path):
    data = Path(path).read_bytes()
    if len(data) < 12 or len(data) % 4:
        raise ValueError(f'invalid MERL length: {path}')
    words = list(struct.unpack(f'>{len(data) // 4}I', data))
    cookie, end_module, end_code = words[:3]
    if cookie != 0x10000002 or end_module != len(data) or not 12 <= end_code <= end_module or end_code % 4:
        raise ValueError(f'invalid MERL header: {path}')
    code, relocations, exports, references = words[3:end_code // 4], [], {}, []
    cursor = end_code // 4
    while cursor < len(words):
        kind, location = words[cursor:cursor + 2]
        cursor += 2
        if location % 4 or not 12 <= location <= end_code:
            raise ValueError(f'invalid MERL location: {path}')
        if kind == 1:
            if location == end_code:
                raise ValueError('relocation outside code')
            relocations.append(location)
        elif kind in (5, 17):
            count = words[cursor]
            cursor += 1
            if cursor + count > len(words):
                raise ValueError('truncated MERL symbol')
            name = ''.join(chr(ch) for ch in words[cursor:cursor + count])
            cursor += count
            if kind == 5:
                if name in exports:
                    raise ValueError(f'duplicate export: {name}')
                exports[name] = location
            else:
                if location == end_code:
                    raise ValueError('reference outside code')
                references.append((location, name))
        else:
            raise ValueError(f'unknown MERL record: {kind}')
    return code, relocations, exports, references


def link(modules):
    output, symbols, references = [], {}, []
    for code, relocations, exports, imports in modules:
        base = 4 * len(output)
        code = code.copy()
        for location in relocations:
            index = (location - 12) // 4
            code[index] = (code[index] + base - 12) & 0xffffffff
        for name, location in exports.items():
            if name in symbols:
                raise ValueError(f'duplicate linked symbol: {name}')
            symbols[name] = base + location - 12
        references.extend(((base + location - 12) // 4, name) for location, name in imports)
        output.extend(code)
    for index, name in references:
        if name not in symbols:
            raise ValueError(f'unresolved import: {name}')
        output[index] = symbols[name]
    return struct.pack(f'>{len(output)}I', *output)


if __name__ == '__main__':
    try:
        if len(sys.argv) < 3:
            raise ValueError('Usage: mips_tools.py source.asm library.merl [...]')
        modules = [assemble(sys.argv[1])] + [read_merl(path) for path in sys.argv[2:]]
        sys.stdout.buffer.write(link(modules))
    except (ValueError, KeyError, IndexError, OSError, struct.error) as error:
        print(f'ERROR: {error}', file=sys.stderr)
        sys.exit(1)
