#include "imu.h"
#include "rm67162.h"
#include "pins_config.h"
#include "display.h"
#include <Wire.h>

static IMUType activeIMU = IMU_NONE;
static uint8_t imuAddress = 0;
static uint8_t currentRotation = 1; // Default: Landscape (1)
static bool autoRotateEnabled = true;
static bool gesturesEnabled = true;
static const char* imuName = "None";

// Filter / Debounce state for orientation
static uint8_t pendingRotation = 1;
static uint8_t debounceCount = 0;
static uint32_t lastCheckMs = 0;
static uint32_t lastOrientMs = 0;

// Gesture Detector State (Double-Tap & Shake)
static int32_t lastAccelMag = 0;
static uint32_t firstTapMs = 0;
static uint32_t lastTapDetectedMs = 0;
static uint8_t tapCount = 0;
static int lastShakeDir = 0;
static uint32_t lastShakeDirMs = 0;
static uint32_t lastShakeActionMs = 0;
static uint8_t shakeCount = 0;

// External callback in main.cpp for cache purge
extern void flushDnsCacheFromGesture();

static bool writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return (Wire.endTransmission() == 0);
}

static uint8_t readReg(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return 0xFF;
    if (Wire.requestFrom((uint8_t)addr, (uint8_t)1) == 1) {
        return Wire.read();
    }
    return 0xFF;
}

static bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)addr, (uint8_t)len) == (int)len) {
        for (size_t i = 0; i < len; i++) {
            buffer[i] = Wire.read();
        }
        return true;
    }
    return false;
}

bool initIMU() {
    Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL, 400000);
    delay(20);

    Serial.println("[imu] Scanning I2C bus (SDA=3, SCL=2) for sensors...");
    bool foundAny = false;
    for (uint8_t a = 1; a < 127; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[imu] Found I2C device at 0x%02X\n", a);
            foundAny = true;
        }
    }
    if (!foundAny) {
        Serial.println("[imu] No I2C devices found on bus.");
    }

    // 1. Probe QMI8658 / QMI8658C (0x6B / 0x6A) - Common on LilyGO T-Display-S3
    for (uint8_t addr : {0x6B, 0x6A}) {
        uint8_t who = readReg(addr, 0x00);
        if (who == 0x05) {
            writeReg(addr, 0x01, 0x60); // CTRL1: internal clock, auto address increment
            writeReg(addr, 0x02, 0x24); // CTRL2: Accel 4g, 250Hz ODR
            writeReg(addr, 0x03, 0x44); // CTRL3: Gyro 512dps, 250Hz ODR
            writeReg(addr, 0x08, 0x03); // CTRL7: Enable Accel & Gyro
            activeIMU = IMU_QMI8658;
            imuAddress = addr;
            imuName = "QMI8658";
            Serial.printf("[imu] Initialized %s at 0x%02X (WHO_AM_I=0x05)\n", imuName, addr);
            return true;
        }
    }

    // 2. Probe MPU6050 / MPU6500 / MPU9250 (0x68 / 0x69)
    for (uint8_t addr : {0x68, 0x69}) {
        uint8_t who = readReg(addr, 0x75);
        if (who == 0x68 || who == 0x70 || who == 0x71 || who == 0x72 || who == 0x73 || who == 0x98) {
            writeReg(addr, 0x6B, 0x00); // Wake up MPU
            delay(10);
            activeIMU = IMU_MPU6050;
            imuAddress = addr;
            imuName = "MPU6050/6500";
            Serial.printf("[imu] Initialized %s at 0x%02X (WHO_AM_I=0x%02X)\n", imuName, addr, who);
            return true;
        }
    }

    // 3. Probe QMA6100P / QMA7981 (0x12)
    {
        uint8_t who = readReg(0x12, 0x00);
        if (who == 0x90 || who == 0x01) {
            writeReg(0x12, 0x11, 0x80); // Active mode
            writeReg(0x12, 0x10, 0x01); // 100Hz ODR
            activeIMU = IMU_QMA6100;
            imuAddress = 0x12;
            imuName = "QMA6100P";
            Serial.printf("[imu] Initialized %s at 0x12\n", imuName);
            return true;
        }
    }

    // 4. Probe LIS3DH / BMA423 (0x18 / 0x19)
    for (uint8_t addr : {0x18, 0x19}) {
        uint8_t whoLis = readReg(addr, 0x0F);
        if (whoLis == 0x33) {
            writeReg(addr, 0x20, 0x57); // 100Hz, normal mode, XYZ on
            activeIMU = IMU_LIS3DH;
            imuAddress = addr;
            imuName = "LIS3DH";
            Serial.printf("[imu] Initialized %s at 0x%02X\n", imuName, addr);
            return true;
        }
        uint8_t whoBma = readReg(addr, 0x00);
        if (whoBma == 0x1D) {
            writeReg(addr, 0x7D, 0x04); // acc_enable
            activeIMU = IMU_BMA423;
            imuAddress = addr;
            imuName = "BMA423";
            Serial.printf("[imu] Initialized %s at 0x%02X\n", imuName, addr);
            return true;
        }
    }

    // 5. Probe LSM6DS3 / LSM6DSL (0x6A / 0x6B)
    for (uint8_t addr : {0x6A, 0x6B}) {
        uint8_t who = readReg(addr, 0x0F);
        if (who == 0x69 || who == 0x6A || who == 0x6C) {
            writeReg(addr, 0x10, 0x40); // 104Hz, 2g
            activeIMU = IMU_LSM6DS;
            imuAddress = addr;
            imuName = "LSM6DS";
            Serial.printf("[imu] Initialized %s at 0x%02X\n", imuName, addr);
            return true;
        }
    }

    // 6. Probe ADXL345 (0x53)
    {
        uint8_t who = readReg(0x53, 0x00);
        if (who == 0xE5) {
            writeReg(0x53, 0x2D, 0x08); // Measurement mode
            activeIMU = IMU_ADXL345;
            imuAddress = 0x53;
            imuName = "ADXL345";
            Serial.printf("[imu] Initialized %s at 0x53\n", imuName);
            return true;
        }
    }

    Serial.println("[imu] No supported gravity sensor found. Manual 180-deg flip active via Button 2 / Web UI.");
    activeIMU = IMU_NONE;
    imuName = "Manual / None";
    return false;
}

static bool readRawAccel(int16_t* ax, int16_t* ay, int16_t* az) {
    uint8_t raw[6];
    if (activeIMU == IMU_QMI8658) {
        if (!readBytes(imuAddress, 0x35, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    } else if (activeIMU == IMU_MPU6050) {
        if (!readBytes(imuAddress, 0x3B, raw, 6)) return false;
        *ax = (int16_t)((raw[0] << 8) | raw[1]);
        *ay = (int16_t)((raw[2] << 8) | raw[3]);
        *az = (int16_t)((raw[4] << 8) | raw[5]);
        return true;
    } else if (activeIMU == IMU_QMA6100) {
        if (!readBytes(imuAddress, 0x01, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    } else if (activeIMU == IMU_LIS3DH) {
        if (!readBytes(imuAddress, 0x28 | 0x80, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    } else if (activeIMU == IMU_BMA423) {
        if (!readBytes(imuAddress, 0x12, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    } else if (activeIMU == IMU_LSM6DS) {
        if (!readBytes(imuAddress, 0x28, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    } else if (activeIMU == IMU_ADXL345) {
        if (!readBytes(imuAddress, 0x32, raw, 6)) return false;
        *ax = (int16_t)((raw[1] << 8) | raw[0]);
        *ay = (int16_t)((raw[3] << 8) | raw[2]);
        *az = (int16_t)((raw[5] << 8) | raw[4]);
        return true;
    }
    return false;
}

void updateIMU() {
    if (activeIMU == IMU_NONE) return;
    uint32_t now = millis();
    if (now - lastCheckMs < 25) return; // Sample at ~40Hz for responsive gesture recognition
    lastCheckMs = now;

    int16_t ax = 0, ay = 0, az = 0;
    if (!readRawAccel(&ax, &ay, &az)) return;

    // 1. Auto-rotation (Filtered at ~6Hz)
    if (autoRotateEnabled && (now - lastOrientMs >= 160)) {
        lastOrientMs = now;
        int32_t val = (abs(ay) > abs(ax)) ? ay : ax;
        const int32_t THRESHOLD = 3500; // ~0.25G threshold to avoid jitter

        uint8_t detected = currentRotation;
        if (val > THRESHOLD) {
            detected = 1; // Normal Landscape
        } else if (val < -THRESHOLD) {
            detected = 3; // Inverted Landscape (180 deg)
        }

        if (detected != currentRotation) {
            if (detected == pendingRotation) {
                if (++debounceCount >= 3) {
                    currentRotation = detected;
                    lcd_setRotation(currentRotation);
                    char msg[32];
                    snprintf(msg, sizeof(msg), "FLIP: %s", (currentRotation == 1) ? "NORMAL (0)" : "INVERTED (180)");
                    displayToast(msg, 1500);
                    debounceCount = 0;
                }
            } else {
                pendingRotation = detected;
                debounceCount = 1;
            }
        } else {
            debounceCount = 0;
        }
    }

    // 2. Gesture Detection: Tap-Tap & Shake
    if (!gesturesEnabled) return;

    int32_t mag = (int32_t)sqrt((int32_t)ax * ax + (int32_t)ay * ay + (int32_t)az * az);
    int32_t delta = abs(mag - lastAccelMag);
    lastAccelMag = mag;

    // A. Double-Tap Detection (輕敲機身兩下 -> 翻至下一頁)
    const int32_t TAP_THRESHOLD = 4600;
    if (delta > TAP_THRESHOLD && (now - lastTapDetectedMs > 80)) {
        if (tapCount == 0) {
            tapCount = 1;
            firstTapMs = now;
            lastTapDetectedMs = now;
        } else if (tapCount == 1 && (now - firstTapMs >= 110 && now - firstTapMs <= 550)) {
            tapCount = 0;
            lastTapDetectedMs = now;
            nextPage();
            displayToast("👆 TAP-TAP: NEXT PAGE", 1500);
            Serial.println("[imu] Double-Tap detected! Switched page.");
        }
    }
    if (tapCount == 1 && (now - firstTapMs > 600)) {
        tapCount = 0;
    }

    // B. Shake Detection (用力搖晃 -> 清空 DNS 快取)
    int curDir = (ax > 5500) ? 1 : ((ax < -5500) ? -1 : 0);
    if (curDir != 0 && curDir != lastShakeDir && (now - lastShakeDirMs < 350)) {
        lastShakeDir = curDir;
        lastShakeDirMs = now;
        shakeCount++;
        if (shakeCount >= 4 && (now - lastShakeActionMs > 2500)) {
            shakeCount = 0;
            lastShakeActionMs = now;
            flushDnsCacheFromGesture();
            displayToast("⚡ SHAKE: CACHE PURGED!", 2000);
            Serial.println("[imu] Shake detected! DNS cache purged.");
        }
    } else if (curDir != 0 && curDir != lastShakeDir) {
        lastShakeDir = curDir;
        lastShakeDirMs = now;
        shakeCount = 1;
    }
    if (now - lastShakeDirMs > 650) {
        shakeCount = 0;
    }
}

void flipScreen180() {
    currentRotation = (currentRotation == 1) ? 3 : 1;
    lcd_setRotation(currentRotation);
    char msg[32];
    snprintf(msg, sizeof(msg), "SCREEN: %s", (currentRotation == 1) ? "NORMAL (0)" : "INVERTED (180)");
    displayToast(msg, 2000);
}

uint8_t getCurrentRotation() {
    return currentRotation;
}

void setAutoRotate(bool enabled) {
    autoRotateEnabled = enabled;
}

bool isAutoRotateEnabled() {
    return autoRotateEnabled;
}

void setGesturesEnabled(bool enabled) {
    gesturesEnabled = enabled;
}

bool isGesturesEnabled() {
    return gesturesEnabled;
}

const char* getIMUName() {
    return imuName;
}
