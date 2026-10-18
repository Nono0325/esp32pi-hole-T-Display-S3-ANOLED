#pragma once

#include <Arduino.h>
#include <LittleFS.h>

// Identify hardware manufacturer or device type based on MAC address
String getDeviceVendor(const uint8_t* mac);

// Load, save and get user-defined device aliases (persisted in LittleFS /aliases.txt)
void initDeviceManager();
void setDeviceAlias(const String& ipStr, const String& alias);
String getDeviceAlias(uint32_t ip);

// Get composite human-friendly device name (Alias > Vendor > Fallback)
String getDeviceDisplayName(uint32_t ip, const uint8_t* mac);
