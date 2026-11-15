#!/usr/bin/env python3
"""Assign the selected packages without rewriting schematic geometry."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
path = root / "PCB/Cott Keyboard/Cott Keyboard.kicad_sch"
assignments = {}
for i in range(1, 16):
    assignments[f"C{i}"] = ("Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder", None)
for i in range(1, 29):
    assignments[f"D{i}"] = ("Diode_SMD:D_SOD-123", "1N4148W")
    assignments[f"SW{i}"] = ("Cott:SW_Omron_D2F-01FL_SPST_NO", "D2F-01FL")
assignments["R1"] = ("Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder", None)
assignments["J2"] = ("Connector_Wire:SolderWire-0.25sqmm_1x05_P4.2mm_D0.65mm_OD1.7mm_Relief", None)

def assign(match):
    block = match[0]
    ref = re.search(r'\(property "Reference" "([^"]+)"', block)[1]
    if ref not in assignments:
        return block
    footprint, value = assignments[ref]
    block = re.sub(r'(\(property "Footprint" )"[^"]*"', lambda m: m[1] + '"' + footprint + '"', block, count=1)
    if value:
        block = re.sub(r'(\(property "Value" )"[^"]*"', lambda m: m[1] + '"' + value + '"', block, count=1)
    return block

path.write_text(re.sub(r'\n\t\(symbol\n.*?\n\t\)', assign, path.read_text(), flags=re.S))
print(f"Assigned {len(assignments)} component footprints")
