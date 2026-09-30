#include "mpu6050mod.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mpu6050mod {

static const char *const TAG = "mpu6050mod";

static const uint8_t REGISTER_CONFIG = 0x1A;  // EXT_SYNC_SET (bits 3-5), DLPF_CFG (bits 0-2)
static const uint8_t REGISTER_GYRO_CONFIG = 0x1B;
static const uint8_t REGISTER_ACCEL_CONFIG = 0x1C;
static const uint8_t REGISTER_ACCEL_XOUT_H = 0x3B;
static const uint8_t REGISTER_POWER_MANAGEMENT_1 = 0x6B;
static const uint8_t REGISTER_WHO_AM_I = 0x75;

static const uint8_t CLOCK_SOURCE_X_GYRO = 0b001;
static const uint8_t SCALE_2000_DPS = 0b11;
static const float SCALE_DPS_PER_DIGIT_2000 = 0.060975f;  // 1 / 16.4
static const uint8_t RANGE_2G = 0b00;
static const float RANGE_PER_DIGIT_2G = 0.000061f;  // 1 / 16384
static const uint8_t BIT_SLEEP_ENABLED = 6;
static const uint8_t BIT_TEMPERATURE_DISABLED = 3;
static const float GRAVITY_EARTH = 9.80665f;

// Accelerometer bandwidth per DLPF_CFG (MPU-6000/6050 register map, 4.3)
static const uint16_t DLPF_BANDWIDTH_HZ[] = {260, 184, 94, 44, 21, 10, 5};

void MPU6050ModComponent::setup() {
  uint8_t who_am_i;
  if (!this->read_byte(REGISTER_WHO_AM_I, &who_am_i) ||
      (who_am_i != 0x68 && who_am_i != 0x70 && who_am_i != 0x98)) {
    this->mark_failed();
    return;
  }

  // Power management: clock source X-gyro, wake up, temperature sensor on
  uint8_t power_management;
  if (!this->read_byte(REGISTER_POWER_MANAGEMENT_1, &power_management)) {
    this->mark_failed();
    return;
  }
  power_management &= 0b11111000;
  power_management |= CLOCK_SOURCE_X_GYRO;
  power_management &= ~(1 << BIT_SLEEP_ENABLED);
  power_management &= ~(1 << BIT_TEMPERATURE_DISABLED);
  if (!this->write_byte(REGISTER_POWER_MANAGEMENT_1, power_management)) {
    this->mark_failed();
    return;
  }

  // Digital low-pass filter: keep EXT_SYNC_SET, set DLPF_CFG
  uint8_t config;
  if (!this->read_byte(REGISTER_CONFIG, &config)) {
    this->mark_failed();
    return;
  }
  config &= 0b11111000;
  config |= (this->dlpf_ & 0b111);
  ESP_LOGV(TAG, "  CONFIG: 0b" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(config));
  if (!this->write_byte(REGISTER_CONFIG, config)) {
    this->mark_failed();
    return;
  }

  // Gyro range +-2000 deg/s
  uint8_t gyro_config;
  if (!this->read_byte(REGISTER_GYRO_CONFIG, &gyro_config)) {
    this->mark_failed();
    return;
  }
  gyro_config &= 0b11100111;
  gyro_config |= SCALE_2000_DPS << 3;
  if (!this->write_byte(REGISTER_GYRO_CONFIG, gyro_config)) {
    this->mark_failed();
    return;
  }

  // Accel range +-2 g (finest resolution)
  uint8_t accel_config;
  if (!this->read_byte(REGISTER_ACCEL_CONFIG, &accel_config)) {
    this->mark_failed();
    return;
  }
  accel_config &= 0b11100111;
  accel_config |= (RANGE_2G << 3);
  if (!this->write_byte(REGISTER_ACCEL_CONFIG, accel_config)) {
    this->mark_failed();
    return;
  }
}

void MPU6050ModComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MPU6050mod:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with MPU6050 failed!");
  }
  ESP_LOGCONFIG(TAG, "  DLPF: %u Hz (DLPF_CFG=%u)", DLPF_BANDWIDTH_HZ[this->dlpf_ & 0b111], this->dlpf_);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Acceleration X", this->accel_x_sensor_);
  LOG_SENSOR("  ", "Acceleration Y", this->accel_y_sensor_);
  LOG_SENSOR("  ", "Acceleration Z", this->accel_z_sensor_);
  LOG_SENSOR("  ", "Gyro X", this->gyro_x_sensor_);
  LOG_SENSOR("  ", "Gyro Y", this->gyro_y_sensor_);
  LOG_SENSOR("  ", "Gyro Z", this->gyro_z_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void MPU6050ModComponent::update() {
  uint16_t raw_data[7];
  if (!this->read_bytes_16(REGISTER_ACCEL_XOUT_H, raw_data, 7)) {
    this->status_set_warning();
    return;
  }
  auto *data = reinterpret_cast<int16_t *>(raw_data);

  float accel_x = data[0] * RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
  float accel_y = data[1] * RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
  float accel_z = data[2] * RANGE_PER_DIGIT_2G * GRAVITY_EARTH;
  float temperature = data[3] / 340.0f + 36.53f;
  float gyro_x = data[4] * SCALE_DPS_PER_DIGIT_2000;
  float gyro_y = data[5] * SCALE_DPS_PER_DIGIT_2000;
  float gyro_z = data[6] * SCALE_DPS_PER_DIGIT_2000;

  ESP_LOGV(TAG, "accel={x=%.3f, y=%.3f, z=%.3f} m/s², gyro={x=%.3f, y=%.3f, z=%.3f} °/s, temp=%.2f °C", accel_x,
           accel_y, accel_z, gyro_x, gyro_y, gyro_z, temperature);

  if (this->accel_x_sensor_ != nullptr)
    this->accel_x_sensor_->publish_state(accel_x);
  if (this->accel_y_sensor_ != nullptr)
    this->accel_y_sensor_->publish_state(accel_y);
  if (this->accel_z_sensor_ != nullptr)
    this->accel_z_sensor_->publish_state(accel_z);
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temperature);
  if (this->gyro_x_sensor_ != nullptr)
    this->gyro_x_sensor_->publish_state(gyro_x);
  if (this->gyro_y_sensor_ != nullptr)
    this->gyro_y_sensor_->publish_state(gyro_y);
  if (this->gyro_z_sensor_ != nullptr)
    this->gyro_z_sensor_->publish_state(gyro_z);

  this->status_clear_warning();
}

}  // namespace mpu6050mod
}  // namespace esphome
