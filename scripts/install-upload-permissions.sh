#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
install -m 0644 "$script_dir/70-cott-keyboard.rules" /etc/udev/rules.d/70-cott-keyboard.rules
udevadm control --reload-rules
udevadm trigger --action=change --subsystem-match=tty
udevadm trigger --action=change --subsystem-match=usb
