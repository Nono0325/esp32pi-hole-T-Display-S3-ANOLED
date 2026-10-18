#pragma once

#include <Arduino.h>

#define MAX_SPECTRUM_APS 28

struct WifiApInfo {
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    uint8_t bssid[6];
    uint8_t encType;
    bool isCurrent;
};

struct SpectrumData {
    uint8_t apCount;
    WifiApInfo aps[MAX_SPECTRUM_APS];
    uint8_t chCount[15];     // AP count per channel (1..14)
    uint8_t chScore[15];     // 0..100 congestion percentage per channel
    uint8_t bestChannel;     // Recommended non-overlapping channel (1, 6, or 11)
    uint8_t currentChannel;  // Current connection channel
    bool scanning;
    uint32_t lastScanMs;
    uint16_t sweepAngle;     // Visual radar beam sweep angle (0..360)
};

// Initialize spectrum scanner
void initSpectrumRadar();

// Trigger non-blocking asynchronous WiFi channel scan
bool triggerSpectrumScan();

// Periodic update in loop() to handle scan completion without blocking
void updateSpectrumScan();

// Check if scan is currently in progress
bool isSpectrumScanning();

// Get read-only reference to latest spectrum analysis
const SpectrumData& getSpectrumData();

// Build JSON payload of spectrum data for Web UI
String getSpectrumJson();
