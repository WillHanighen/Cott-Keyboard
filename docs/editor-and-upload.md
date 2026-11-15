# VSCodium diagnostics and USB upload access

## Refresh clangd

From the repository root:

```sh
python3 scripts/configure_clangd.py
```

Then run **clangd: Restart language server** in VSCodium.

The script regenerates PlatformIO's compilation database and adds C and C++
system headers queried from each actual compiler. The .clangd file selects the
Xtensa target and prevents desktop Linux system headers from mixing with the
ESP32 headers. Unsupported GCC-only flags are removed for clangd only.
Compiler diagnostics from the real firmware build remain enabled.

Run this script again after changing PlatformIO dependencies, build flags, or
toolchain versions. Plain `pio run -t compiledb` overwrites the augmented
database, so use the script to refresh it.

Workspace settings enable clangd indexing and disable the Microsoft C/C++
IntelliSense engine to avoid competing diagnostic providers. The screenshot
shows both the repository and the nested firmware directory as workspace roots.
One repository root is sufficient; removing the duplicate folder is optional.

Verify parsing with:

```sh
clangd --check="firmware/Cottage Keyboard/src/main.cpp" --tweaks=
```

The empty tweaks list disables clangd's refactoring self-tests during this
command. It does not disable diagnostics in the editor.

## USB permissions on this machine

CachyOS uses udev and systemd-logind. The prepared rules in
`scripts/70-cott-keyboard.rules` grant the active local desktop user access to
native Espressif USB devices and common CP210x, CH340, and FTDI USB-UART bridges.
They do not make serial devices writable by every account.

Install and reload the rules with:

```sh
sudo sh scripts/install-upload-permissions.sh
```

Reconnect the board after installation if it was already plugged in. These rules
use the active desktop session's ACLs and do not require adding the user to a
serial group. A headless SSH session needs a separate group-based setup.

No board was connected during setup. Permissions can be installed and the rules
can be validated, but an actual upload cannot be verified until hardware exists.
Without a board, an upload failure to find a serial port is expected.

Do not run PlatformIO itself with sudo. If the board later appears but the port
is busy, close the serial monitor before uploading.
