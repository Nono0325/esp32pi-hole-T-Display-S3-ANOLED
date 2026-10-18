#pragma once

#include <Arduino.h>
#include <WiFi.h>

// Initialize Display, Sprites, Buttons & Touch
void initDisplay();

// Call periodically in loop() to handle buttons, touch & screen refresh
void updateDisplay(bool force = false);

// Show a temporary message toast on screen (e.g., "PAUSED (5m)", "UPDATING...")
void displayToast(const char* message, uint32_t durationMs = 2500);

// Record query activity for the history sparkline graph and rate tracking
void recordQueryActivity(bool blocked, uint16_t bytes = 64);

// Live traffic rate and throughput getters
float getCurrentQPS();
float getCurrentBPS();
float getPeakQPS();
float getPeakBPS();

// Set display brightness (0 - 255)
void setDisplayBrightness(uint8_t brightness);

// Cycle to next page
void nextPage();

// Set active page directly (0 to 5)
void setDisplayPage(uint8_t page);

// Screen sleep and wakeup controls
void sleepDisplay();
void wakeupDisplay();
bool isDisplaySleeping();

// Auto-sleep timeout in seconds (0 = never, 30, 60, 300, 600)
void setSleepTimeout(uint32_t seconds);
uint32_t getSleepTimeout();

// Reset inactivity timer
void resetActivityTimer();

// Internet Sentinel connectivity status & ping latency
void setInternetStatus(bool online, int32_t rttMs);

// Current page index (0: Overview, 1: DNS Activity, 2: Clock & Weather, 3: Top Blocked, 4: System Health, 5: QR Portal, 6: Cyber Sentinel)
extern uint8_t currentPage;

// Play / Replay NonoWuW Cyber Boot Animation
void playBootAnimation();
void replayBootAnimation();
