# mpu6050mod

Kopie der offiziellen ESPHome-Komponente
[`mpu6050`](https://github.com/esphome/esphome/tree/dev/esphome/components/mpu6050)
mit einer Ergänzung (markiert mit `//MOD Busse`, Autor der Mod unbekannt):

- **DLPF 21 Hz:** Beim Setup wird Register `0x1A` (CONFIG) auf `DLPF_CFG = 0b100` gesetzt.
  Die offizielle Komponente lässt das Register auf dem Default (~260 Hz Bandbreite).

Grund: Das Womo wird bei **laufendem Motor** nivelliert. Der Hardware-Tiefpass filtert
Motorvibrationen vor der Abtastung (50 ms = 20 Hz), sonst erscheinen sie durch Aliasing
als Scheinneigung. Die Software-Mittelung in `mpu6050.yaml` allein reicht dafür nicht.

Einbindung (`mpu6050.yaml`):

```yaml
external_components:
  - source:
      type: local
      path: custom_mpu
sensor:
  - platform: mpu6050mod
```

Lizenz: ESPHome-C++-Code GPLv3, Python-Code MIT (wie das Original).
