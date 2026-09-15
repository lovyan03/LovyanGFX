#!/usr/bin/env python3
"""Consistency checks for a library built as a single translation unit.

The implementation lives in *.inl files that a single hub *.cpp includes,
directly or through other *.inl files. This script checks that the hub and
the *.inl files agree:

  1. every *.inl include reachable from the hub names an existing file under
     the source root, no file includes the same *.inl twice, and *.inl files
     are included with the quoted form only
  2. every *.inl under the source root is reachable from the hub
  3. every *.inl starts with  #ifndef <MACRO> / #error / #endif  so it cannot
     be included by user code on its own
  4. no *.c / *.cpp other than the hub exists under the source root (a stray
     one would compile as a separate translation unit and defeat the purpose)
  5. (--compile) every *.inl the hub includes directly compiles on its own with
     only the guard macro defined, so no file silently depends on what an
     earlier include brought in (files included by other *.inl files are
     fragments of those and are not compiled separately)

The checks are lexical: comments, string literals and line splices are
handled like the preprocessor does, but #if conditions are not evaluated, so
an include inside a disabled #if block still counts as reachable, a file
included from several places (the platform directories each include the
same bit-bang helpers, under mutually exclusive conditions) is not reported
as a duplicate, and a platform file that compiles to nothing on the host
still passes the compile check. The script guards against mistakes, not
against code written to evade it.

Usage:
  check_inl_sources.py --hub src/lgfx/v1/lgfx_v1.cpp --root src/lgfx/v1 \
      --macro LGFX_V1_IMPLEMENTATION [--compile CXX -std=c++17 -DLGFX_SDL -Isrc ...]

Everything after --compile is the compiler command line; the script appends
-fsyntax-only -x c++ -D<MACRO> and the file.
"""

import argparse
import concurrent.futures
import os
import re
import subprocess
import sys

WS = r'[ \t\f\v]'
INCLUDE_RE = re.compile(r'^' + WS + r'*#' + WS + r'*include' + WS + r'*(?:"([^"\n]+)"|<([^>\n]+)>)' + WS + r'*$', re.M)
DIRECTIVE_RE = re.compile(r'^' + WS + r'*#' + WS + r'*(\w+)(.*)$', re.M)
RAW_STRING_RE = re.compile(r'(?:u8|u|U|L)?R"([^()\\ \t\f\v\n]{0,16})\(')


def digit_separator_at(text, i):
    """True when the quote at text[i] separates digits of a numeric literal (1'000, 0xFF'FF)."""
    k = i - 1
    while k >= 0 and (text[k].isalnum() or text[k] in '_.'):
        k -= 1
    token = text[k + 1:i].lstrip('.')
    return bool(token) and token[0].isdigit() and i + 1 < len(text) and (text[i + 1].isalnum())


def raw_string_at(text, i):
    """Match object when the quote at text[i] opens a raw string literal, else None."""
    for start in (i - 1, i - 2, i - 3):
        if start < 0:
            break
        if start > 0 and (text[start - 1].isalnum() or text[start - 1] == '_'):
            continue
        m = RAW_STRING_RE.match(text, start)
        if m and text[m.start():].startswith(m.group(0)) and m.group(0).index('"') == i - start:
            return m
    return None


def read_code(path):
    """File contents prepared for line-based directive matching.

    Line splices are joined (except inside raw string literals), comments
    are replaced by a space, raw string literals are blanked, and ordinary
    string and character literals are copied through (a comment opener
    inside one is not a comment). Newlines are kept so each directive still
    sits on its own line.
    """
    with open(path, encoding='utf-8', errors='replace') as f:
        text = f.read().replace('\r\n', '\n')
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith('\\\n', i):
            i += 2
        elif text.startswith('//', i):
            j = text.find('\n', i)
            while 0 < j and text[j - 1] == '\\':
                j = text.find('\n', j + 1)
            i = n if j < 0 else j
        elif c == "'" and digit_separator_at(text, i):
            i += 1
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(' ' + '\n' * text.count('\n', i, j))
            i = j
        elif c == '"' and (m := raw_string_at(text, i)):
            # raw string literal R"delim( ... )delim": blank it, keep its newlines
            close = ')' + m.group(1) + '"'
            j = text.find(close, m.end())
            j = n if j < 0 else j + len(close)
            out.append('""' + '\n' * text.count('\n', i, j))
            i = j
        elif c in '"\'':
            # ordinary string or character literal: copy it. Line splices are removed
            # before escape sequences are read, as in translation phase 2.
            lit = [c]
            j = i + 1
            while j < n:
                if text.startswith('\\\n', j):
                    j += 2
                    continue
                ch = text[j]
                if ch == '\n':
                    break
                j += 1
                if ch == c:
                    lit.append(c)
                    break
                if ch == '\\':
                    while text.startswith('\\\n', j):
                        j += 2
                    if j < n and text[j] != '\n':
                        lit.append('\\' + text[j])
                        j += 1
                    continue
                lit.append(ch)
            out.append(''.join(lit))
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def rel(path, base):
    return os.path.relpath(path, base).replace(os.sep, '/')


def walk(root, exts):
    for dirpath, _dirs, files in os.walk(root):
        for name in sorted(files):
            if os.path.splitext(name)[1] in exts:
                yield os.path.normpath(os.path.join(dirpath, name))


def inl_includes(path):
    """(form, include text) for every *.inl include directive in the file.

    form is '"' or '<' for a well-formed directive, and '?' with the raw
    directive text when an #include mentions .inl but does not parse.
    """
    found = []
    code = read_code(path)
    for m in DIRECTIVE_RE.finditer(code):
        if m.group(1) != 'include':
            continue
        inc = INCLUDE_RE.match(code, m.start())
        if inc:
            form, name = ('"', inc.group(1)) if inc.group(1) is not None else ('<', inc.group(2))
            if name.endswith('.inl'):
                found.append((form, name))
        elif '.inl' in m.group(2):
            found.append(('?', m.group(0).strip()))
    return found


def is_under(path, root):
    """True when the file (symlinks resolved) lies under root."""
    real_root = os.path.realpath(root)
    return os.path.commonpath([os.path.realpath(path), real_root]) == real_root


def check_reachability(hub, root, problems):
    """Walks the *.inl include graph from the hub.

    Returns (direct, reachable): the files the hub includes directly, in hub
    order, and every *.inl reachable from the hub.
    """
    direct = []
    reachable = set()
    visited = set()

    def visit(src, chain):
        key = os.path.realpath(src)
        if key in visited:
            return
        visited.add(key)
        seen_here = set()
        for form, inc in inl_includes(src):
            if form == '?':
                problems.append(f'{rel(src, root)}: malformed include directive: {inc}')
                continue
            if form == '<':
                problems.append(f'{rel(src, root)}: includes <{inc}>; use the quoted form for *.inl files')
                continue
            path = os.path.normpath(os.path.join(os.path.dirname(src), inc))
            if not os.path.isfile(path):
                problems.append(f'{rel(src, root)}: includes missing file "{inc}"')
                continue
            if not is_under(path, root):
                problems.append(f'{rel(src, root)}: includes "{inc}", which is outside {rel(root, os.getcwd())}')
                continue
            real = os.path.realpath(path)
            if real in seen_here:
                problems.append(f'{rel(src, root)}: includes "{inc}" twice')
                continue
            seen_here.add(real)
            if real in chain:
                problems.append(f'{rel(src, root)}: includes "{inc}", which already includes this file (cycle)')
                continue
            if src == hub:
                direct.append(path)
            reachable.add(path)
            visit(path, chain + (real,))

    visit(hub, (os.path.realpath(hub),))
    for inl in walk(root, {'.inl'}):
        if inl not in reachable:
            problems.append(f'{rel(inl, root)}: not reachable from the hub')
    return direct, reachable


def check_guards(root, macro, problems):
    """The first three directives of every *.inl must be #ifndef MACRO / #error / #endif."""
    for inl in walk(root, {'.inl'}):
        directives = [(m.group(1), m.group(2).strip()) for m in DIRECTIVE_RE.finditer(read_code(inl))]
        ok = (len(directives) >= 3
              and directives[0] == ('ifndef', macro)
              and directives[1][0] == 'error'
              and directives[2][0] == 'endif')
        if not ok:
            problems.append(f'{rel(inl, root)}: does not start with "#ifndef {macro}" / "#error" / "#endif"')


def check_stray_sources(root, hub, problems):
    for src in walk(root, {'.c', '.cpp'}):
        if os.path.abspath(src) != os.path.abspath(hub):
            problems.append(f'{rel(src, root)}: source file outside the hub (add it as *.inl to the hub instead)')


def compile_one(cmd, macro, path):
    full = cmd + ['-fsyntax-only', '-x', 'c++', '-D' + macro, path]
    proc = subprocess.run(full, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return path, proc.returncode, proc.stdout


def check_compile(direct, root, macro, cmd, problems):
    workers = os.cpu_count() or 2
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(compile_one, cmd, macro, p) for p in direct]
        for fut in futures:
            path, rc, out = fut.result()
            if rc != 0:
                head = '\n'.join(out.strip().splitlines()[:12])
                problems.append(f'{rel(path, root)}: does not compile on its own\n{head}')


def main():
    argv = sys.argv[1:]
    compile_cmd = None
    if '--compile' in argv:
        i = argv.index('--compile')
        compile_cmd = argv[i + 1:]
        argv = argv[:i]
        if not compile_cmd:
            sys.exit('--compile needs a compiler command line')
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--hub', required=True, help='the *.cpp that includes the *.inl files')
    ap.add_argument('--root', required=True, help='directory whose *.inl files must all be reachable from the hub')
    ap.add_argument('--macro', required=True, help='guard macro the hub defines around its includes')
    args = ap.parse_args(argv)

    hub = os.path.normpath(os.path.abspath(args.hub))
    root = os.path.normpath(os.path.abspath(args.root))
    problems = []

    direct, reachable = check_reachability(hub, root, problems)
    check_guards(root, args.macro, problems)
    check_stray_sources(root, hub, problems)
    if compile_cmd:
        check_compile(direct, root, args.macro, compile_cmd, problems)

    print(f'{rel(hub, root)}: {len(direct)} files included directly, {len(reachable)} reachable, '
          f'{sum(1 for _ in walk(root, {".inl"}))} *.inl under {rel(root, os.getcwd())}'
          + (f', compiled with: {" ".join(compile_cmd)}' if compile_cmd else ''))
    if problems:
        print(f'{len(problems)} problem(s):')
        for p in problems:
            print('  - ' + p.replace('\n', '\n      '))
        sys.exit(1)
    print('OK')


if __name__ == '__main__':
    main()
