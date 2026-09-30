#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace mpu6050mod {

class MPU6050ModComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  /// DLPF_CFG value 0-6 (0 = 260 Hz ... 6 = 5 Hz), see register 0x1A
  void set_dlpf(uint8_t dlpf) { this->dlpf_ = dlpf; }

  void set_accel_x_sensor(sensor::Sensor *s) { this->accel_x_sensor_ = s; }
  void set_accel_y_sensor(sensor::Sensor *s) { this->accel_y_sensor_ = s; }
  void set_accel_z_sensor(sensor::Sensor *s) { this->accel_z_sensor_ = s; }
  void set_temperature_sensor(sensor::Sensor *s) { this->temperature_sensor_ = s; }
  void set_gyro_x_sensor(sensor::Sensor *s) { this->gyro_x_sensor_ = s; }
  void set_gyro_y_sensor(sensor::Sensor *s) { this->gyro_y_sensor_ = s; }
  void set_gyro_z_sensor(sensor::Sensor *s) { this->gyro_z_sensor_ = s; }

 protected:
  uint8_t dlpf_{4};  // 21 Hz

  sensor::Sensor *accel_x_sensor_{nullptr};
  sensor::Sensor *accel_y_sensor_{nullptr};
  sensor::Sensor *accel_z_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *gyro_x_sensor_{nullptr};
  sensor::Sensor *gyro_y_sensor_{nullptr};
  sensor::Sensor *gyro_z_sensor_{nullptr};
};

}  // namespace mpu6050mod
}  // namespace esphome
