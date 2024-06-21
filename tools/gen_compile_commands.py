#!/bin/python3

import sys
import os
import pathlib
import shlex
import json
from dataclasses import dataclass
from typing import Optional
COMPILERS=['gcc', 'clang', 'clang++', 'g++', 'as']
SOURCE_FILE_EXTENSIONS=['.s', '.S', '.c', '.cc']
IGNORED_OPTIONS=['-f no-leading-underscore', '-fno-leading-underscore']

@dataclass
class CompileCommand:
    input_file: str
    args: list[str]
    output_file: Optional[str] = None

commands: list[CompileCommand] = []

for line in sys.stdin.readlines():
    command_args = shlex.split(line)
    if not any ([command_args[0].endswith(c) for c in COMPILERS]):
        continue

    filtered_args = []
    for i, v in enumerate(command_args):
        cv = v + ' ' + command_args[i+1] if i+1 < len(command_args) else None
        if v in IGNORED_OPTIONS or (cv is not None and cv in IGNORED_OPTIONS):
            continue
        filtered_args.append(v)

    files = []
    for i, arg in enumerate(command_args[1:]):
        if arg.startswith("-") or not any([arg.endswith(v) for v in SOURCE_FILE_EXTENSIONS]):
            continue

        files.append(arg)

    for v in files:
        commands.append(CompileCommand(input_file=os.path.abspath(v), args=filtered_args))


print(json.dumps([{ 'file': c.input_file, 'directory': os.getcwd(), 'arguments': c.args } for c in commands]), end="")

