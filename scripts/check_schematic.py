#!/usr/bin/env python3
"""Check schematic connectivity against the firmware's pin assignments."""
import re
import subprocess
import tempfile
from pathlib import Path
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
schematic = root / "PCB/Cott Keyboard/Cott Keyboard.kicad_sch"
source = (root / "firmware/Cottage Keyboard/src/main.cpp").read_text()
with tempfile.TemporaryDirectory() as work:
    netlist = Path(work) / "keyboard.xml"
    subprocess.run(["kicad-cli", "sch", "erc", "--exit-code-violations",
                    "--output", str(Path(work) / "erc.rpt"), str(schematic)], check=True)
    subprocess.run(["kicad-cli", "sch", "export", "netlist", "--format", "kicadxml",
                    "--output", str(netlist), str(schematic)], check=True)
    document = ET.parse(netlist)
nets = {n.attrib["name"]: n.findall("node") for n in document.findall("./nets/net")}
pin_nets = {(n.attrib["ref"], n.attrib["pin"]): name
            for name, nodes in nets.items() for n in nodes}


def gpio(name):
    node = next(n for n in nets[name] if n.attrib["ref"] == "U1")
    return int(re.match(r"GPIO(\d+)", node.attrib["pinfunction"])[1])


def array(name):
    return [int(x) for x in re.search(name + r"\[\] = \{([^}]+)\}", source)[1].split(",")]


assert [gpio(f"/KEY_{i}") for i in range(14)] == array("kColumns")
assert [gpio("/ROW_A"), gpio("/ROW_B")] == array("kRows")
for name, constant in {
    "/JOY_X": "kJoyX", "/JOY_Y": "kJoyY", "/JOY_SW": "kJoySwitch",
    "/OLED_SDA": "kSda", "/OLED_SCL": "kScl", "/LED_DATA": "kLed",
}.items():
    assert gpio(name) == int(re.search(constant + r" = (\d+)", source)[1]), name
# Every column has two diode anodes. Each cathode reaches a switch,
# and the two switches return to distinct scan rows.
for col in range(14):
    diodes = [n.attrib["ref"] for n in nets[f"/KEY_{col}"]
              if n.attrib["ref"].startswith("D")]
    assert len(diodes) == 2
    rows = set()
    for diode in diodes:
        assert pin_nets[diode, "2"] == f"/KEY_{col}"
        cathode = pin_nets[diode, "1"]
        switch = next(n.attrib["ref"] for n in nets[cathode]
                      if n.attrib["ref"].startswith("SW"))
        rows.add(pin_nets[switch, "2"])
    assert rows == {"/ROW_A", "/ROW_B"}
assert pin_nets["U2", "1"] == pin_nets["U2", "3"] == "GND"
assert pin_nets["U2", "5"] == "+5V"
assert pin_nets["U2", "4"] == pin_nets["R1", "1"]
assert pin_nets["R1", "2"] == pin_nets["D29", "4"]
for led in range(29, 42):
    assert pin_nets[f"D{led}", "2"] == pin_nets[f"D{led + 1}", "4"]
for led in range(29, 43):
    assert pin_nets[f"D{led}", "1"] == "+5V"
    assert pin_nets[f"D{led}", "3"] == "GND"
assert [pin_nets["J1", str(i)] for i in range(1, 5)] == [
    "GND", "+3V3", "/OLED_SDA", "/OLED_SCL"]
assert [pin_nets["J2", str(i)] for i in range(1, 6)] == [
    "+3V3", "GND", "/JOY_X", "/JOY_Y", "/JOY_SW"]
missing = [c.attrib["ref"] for c in document.findall("./components/comp")
           if not c.findtext("footprint")]
print("PASS: firmware GPIOs, 14 dual-contact pairs, LED chain, level shifter, connectors")
print(f"Missing footprints ({len(missing)}): {', '.join(missing)}")
