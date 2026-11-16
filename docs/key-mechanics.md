# Key mechanics

Current design decisions, recorded 2026-11-15. Dimensions are design targets;
the mechanism has not been physically tested. No components have been purchased.

## Dimensions

| Measurement | Target |
| --- | --- |
| Overall key length | 85 mm |
| Key width, across neighboring keys | 22 mm |
| Gap between neighboring keys | 3 mm |
| Key pitch, center to center | 25 mm |
| Number of keys | 14 |
| Arrangement | One straight row, no offsets |
| Total key-row width, excluding case margins | 347 mm |

The row width is 14 × 22 mm + 13 × 3 mm = 347 mm.
The distance between the first and last key centers is 13 × 25 mm = 325 mm.
These dimensions do not set the PCB outline or case dimensions.

The 22 mm width and 3 mm gap replace the earlier 20 mm width and 5 mm gap.
Key height and material thickness are not yet specified.

## Mechanism

The case will be FDM printed. The [side-profile sketch](key%20side%20profile.excalidraw)
shows a key pivoting near one end above two lever microswitches on the PCB.
The switches sit at different distances from the pivot, along the key's length.
An LED sits between the switches.

The intended behavior is for one switch to close before the other during a key
press. The interval between closures provides the input for strike velocity.
The project currently specifies Omron D2F-01FL switches; see
[component packages](component-packages.md) for the footprint and contact mapping.

## Unresolved dimensions and fit

- Pivot position and height above the PCB.
- Switch positions, orientations, and contact points against the key.
- Key travel, return mechanism, and travel stops.
- Clearance above the LED throughout the key's travel.

The 85 mm overall length does not specify the pivot-to-tip distance.
The 25 mm key pitch does not specify switch-pad spacing.
Reliable switch sequencing and remaining switch travel require mechanical validation.
