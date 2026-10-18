#pragma once

#include <Arduino.h>

enum IMUType {
    IMU_NONE = 0,
    IMU_MPU6050,    // MPU6050 / MPU6500 / MPU9250 (0x68 / 0x69)
    IMU_QMI8658,    // QMI8658 / QMI8658C (0x6B / 0x6A) - Common on LilyGO T-Display-S3
    IMU_QMA6100,    // QMA6100P / QMA7981 (0x12)
    IMU_LIS3DH,     // LIS3DH / LIS2DH (0x18 / 0x19)
    IMU_BMA423,     // BMA423 / BMA400 (0x18 / 0x19)
    IMU_LSM6DS,     // LSM6DS3 / LSM6DSL (0x6A / 0x6B)
    IMU_ADXL345     // ADXL345 (0x53)
};

// Initialize IMU detection and setup on I2C bus (SDA=3, SCL=2)
bool initIMU();

// Periodic update call to read acceleration, auto-rotation, and gestures (Tap-Tap / Shake)
void updateIMU();

// Flip screen 180 degrees manually (Rotation 1 <-> Rotation 3)
void flipScreen180();

// Get current screen rotation (1: Normal Landscape, 3: Inverted Landscape)
uint8_t getCurrentRotation();

// Enable or disable auto-rotation
void setAutoRotate(bool enabled);
bool isAutoRotateEnabled();

// Enable or disable gesture controls (Tap-Tap / Shake)
void setGesturesEnabled(bool enabled);
bool isGesturesEnabled();

// Get detected IMU model name
const char* getIMUName();
