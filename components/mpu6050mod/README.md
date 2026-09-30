# mpu6050mod

Basiert auf der offiziellen ESPHome-Komponente
[`mpu6050`](https://github.com/esphome/esphome/tree/dev/esphome/components/mpu6050).
Idee Hardware-Tiefpass aus einer Mod («MOD Busse», Autor unbekannt).

## Unterschiede zum Original
- **`dlpf`** (neu): digitaler Tiefpass im Sensor, Register `0x1A` (DLPF_CFG).
  Werte: `260hz`, `184hz`, `94hz`, `44hz`, `21hz` (Default), `10hz`, `5hz`.
  Das Original lässt das Register auf dem Default (260 Hz).
- Eigener Namespace `mpu6050mod` → kollidiert nicht mit der Core-Komponente.
- Messwert-Log pro Update auf VERBOSE (statt DEBUG) – bei 50 ms Intervall sonst 20 Zeilen/s.
- `dump_config` zeigt den aktiven Tiefpass.

## Warum
Das Womo wird bei **laufendem Motor** nivelliert. Der Tiefpass filtert Motorvibrationen im Sensor,
bevor mit 20 Hz (50 ms) abgetastet wird. Ohne ihn erscheinen sie durch Aliasing als Scheinneigung;
die Software-Mittelung in der YAML glättet nur, filtert aber kein Aliasing weg.

## Einbindung
```yaml
external_components:
  - source: github://CzarofAK/mpu6050@v1.0.0
    components: [mpu6050mod]

sensor:
  - platform: mpu6050mod
    address: 0x68
    update_interval: 50ms
    dlpf: 21hz
    accel_x:
      name: "Accel X"
```

Lizenz: C++ GPLv3, Python MIT (wie ESPHome).
