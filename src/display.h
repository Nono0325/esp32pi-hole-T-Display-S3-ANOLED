#pragma once

#include <Arduino.h>
#include <WiFi.h>

// Initialize Display, Sprites, Buttons & Touch
void initDisplay();

// Call periodically in loop() to handle buttons, touch & screen refresh
void updateDisplay(bool force = false);

// Show a temporary message toast on screen (e.g., "PAUSED (5m)", "UPDATING...")
void displayToast(const char* message, uint32_t durationMs = 2500);

// Record query activity for the history sparkline graph
void recordQueryActivity(bool blocked);

// Set display brightness (0 - 255)
void setDisplayBrightness(uint8_t brightness);

// Cycle to next page
void nextPage();

// Current page index (0: Overview, 1: DNS Activity, 2: System Health, 3: QR Portal)
extern uint8_t currentPage;
