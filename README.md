# mpu6050 – WOMO-Nivelliersensor

ESPHome-Konfiguration für den Nivelliersensor im Wohnmobil (FRAM).
Zeigt an, wieviel unter jedes Rad (VL/VR/HL/HR) gelegt werden muss, damit das Womo waagrecht steht.

## Hardware
- D1 Mini (ESP8266)
- MPU6050 über I2C (SDA GPIO4, SCL GPIO5, Adresse 0x68)

## Dateien
| Datei | Inhalt |
|---|---|
| `mpu6050.yaml` | Gerätekonfiguration |
| `components/mpu6050mod/` | External Component: ESPHome-`mpu6050` + einstellbarer Hardware-Tiefpass (Details dort) |
| `.basics.yaml` | kommt aus [esphome_basics](https://github.com/CzarofAK/esphome_basics) (API, OTA, WiFi, Logger, Diagnose) |

## Funktionsweise
- Sensor-Tiefpass (DLPF) 21 Hz, weil bei laufendem Motor nivelliert wird.
- MPU6050 wird alle 50 ms abgetastet, gleitender Mittelwert über 20 Werte (1 s).
- Kalibrierung (Offset/Multiplikator je Achse) und Winkelberechnung laufen 1×/s intern auf dem ESP.
- Einbau-Offsets Roll/Pitch sind `number`-Entitäten auf dem ESP (im Flash gespeichert),
  in HA unter Gerät «mpu6050» → Konfiguration.
- An HA gehen nur Roll, Pitch, Roll/Pitch cm und die Unterleghöhe je Rad.
  Gesendet wird bei Änderung (0.05° bzw. 0.2 cm) oder spätestens alle 60 s.
- Rohwerte zum Kalibrieren stehen im ESPHome-Log (Tag `mpu`, Level DEBUG).

## Fahrzeuggeometrie (Globals)
Spurweite vorne 189 cm, hinten 176 cm, Achsabstand 380 cm.

## Historie
- 2026-09-30 v1.0.0: Komponente als External Component von GitHub, `dlpf` einstellbar, eigener Namespace.
- 2026-09-30: Accel-Template-Sensoren entfernt, Zwischenwerte `internal`, Ausgaben gedrosselt (YouTrack FRA-96).
