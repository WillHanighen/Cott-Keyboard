#!/usr/bin/env python3
"""Refresh PlatformIO's database with system headers from its actual compiler."""
import json
from pathlib import Path
import shlex
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
project = root / "firmware/Cottage Keyboard"
pio = shutil.which("pio") or str(Path.home() / ".platformio/penv/bin/pio")
subprocess.run([pio, "run", "-d", str(project), "-t", "compiledb"], check=True)
path = project / "compile_commands.json"
database = json.loads(path.read_text())
cache = {}
for entry in database:
    args = shlex.split(entry.pop("command")) if "command" in entry else entry["arguments"]
    compiler = args[0]
    if compiler not in cache:
        # PlatformIO sometimes exports a bare executable rather than its full path.
        driver = Path(compiler)
        if not driver.is_absolute():
            candidates = list((Path.home() / ".platformio/packages").glob(f"toolchain-*/bin/{compiler}"))
            if len(candidates) != 1:
                raise RuntimeError(f"Cannot locate compiler {compiler}: {candidates}")
            driver = candidates[0]
        language = "c++" if compiler.endswith(("g++", "c++")) else "c"
        result = subprocess.run([str(driver), "-E", "-v", "-x", language, "-"],
                                input="", capture_output=True, text=True, check=True)
        block = result.stderr.split("#include <...> search starts here:", 1)[1].split("End of search list.", 1)[0]
        includes = [line.strip() for line in block.splitlines() if line.strip()]
        cache[compiler] = (str(driver), includes)
    driver, includes = cache[compiler]
    # Explicit include paths let clangd work without executing a query driver.
    entry.pop("arguments", None)
    entry["command"] = shlex.join([driver, *[flag for inc in includes for flag in ("-isystem", inc)], *args[1:]])
path.write_text(json.dumps(database, indent=2) + "\n")
print(f"Refreshed {len(database)} commands using {len(cache)} compiler configurations")
