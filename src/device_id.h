#pragma once

#include <Arduino.h>
#include <LittleFS.h>

// Identify hardware manufacturer or brand based on MAC address (with rich emoji/branding)
String getDeviceVendor(const uint8_t* mac);

// Pure ASCII brand string suitable for AMOLED screen built-in fonts (no UTF-8 / emoji)
String getDeviceVendorAscii(const uint8_t* mac);

// Load, save and get user-defined device aliases (persisted in LittleFS /aliases.txt)
void initDeviceManager();
void setDeviceAlias(const String& ipStr, const String& alias);
String getDeviceAlias(uint32_t ip);

// Get composite human-friendly device name (Alias > Vendor > Fallback)
String getDeviceDisplayName(uint32_t ip, const uint8_t* mac);
