#include "device_id.h"
#include <WiFi.h>

struct OuiPrefix {
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    const char* vendor;
    const char* ascii;
};

// IEEE OUI database for high-precision device brand identification
static const OuiPrefix VENDOR_TABLE[] = {
    // ==================== Apple (iPhone, iPad, Mac, Watch, Apple TV) ====================
    {0x00, 0x03, 0x93, "🍎 Apple", "Apple"}, {0x00, 0x05, 0x02, "🍎 Apple", "Apple"},
    {0x00, 0x0A, 0x27, "🍎 Apple", "Apple"}, {0x00, 0x0A, 0x95, "🍎 Apple", "Apple"},
    {0x00, 0x0D, 0x93, "🍎 Apple", "Apple"}, {0x00, 0x10, 0xFA, "🍎 Apple", "Apple"},
    {0x00, 0x11, 0x24, "🍎 Apple", "Apple"}, {0x00, 0x14, 0x51, "🍎 Apple", "Apple"},
    {0x00, 0x16, 0xCB, "🍎 Apple", "Apple"}, {0x00, 0x17, 0xF2, "🍎 Apple", "Apple"},
    {0x00, 0x19, 0xE3, "🍎 Apple", "Apple"}, {0x00, 0x1B, 0x63, "🍎 Apple", "Apple"},
    {0x00, 0x1C, 0xB3, "🍎 Apple", "Apple"}, {0x00, 0x1D, 0x4F, "🍎 Apple", "Apple"},
    {0x00, 0x1E, 0x52, "🍎 Apple", "Apple"}, {0x00, 0x1E, 0xC2, "🍎 Apple", "Apple"},
    {0x00, 0x1F, 0x5B, "🍎 Apple", "Apple"}, {0x00, 0x1F, 0xF3, "🍎 Apple", "Apple"},
    {0x00, 0x21, 0xE9, "🍎 Apple", "Apple"}, {0x00, 0x22, 0x41, "🍎 Apple", "Apple"},
    {0x00, 0x23, 0x12, "🍎 Apple", "Apple"}, {0x00, 0x23, 0x32, "🍎 Apple", "Apple"},
    {0x00, 0x23, 0x69, "🍎 Apple", "Apple"}, {0x00, 0x23, 0xDF, "🍎 Apple", "Apple"},
    {0x00, 0x24, 0x36, "🍎 Apple", "Apple"}, {0x00, 0x25, 0x00, "🍎 Apple", "Apple"},
    {0x00, 0x25, 0x4B, "🍎 Apple", "Apple"}, {0x00, 0x25, 0xBC, "🍎 Apple", "Apple"},
    {0x00, 0x26, 0x08, "🍎 Apple", "Apple"}, {0x00, 0x26, 0x4A, "🍎 Apple", "Apple"},
    {0x00, 0x26, 0xB0, "🍎 Apple", "Apple"}, {0x00, 0x26, 0xBB, "🍎 Apple", "Apple"},
    {0x00, 0xCD, 0xFE, "🍎 Apple", "Apple"}, {0x04, 0x0C, 0xCE, "🍎 Apple", "Apple"},
    {0x04, 0x15, 0x52, "🍎 Apple", "Apple"}, {0x04, 0x26, 0x65, "🍎 Apple", "Apple"},
    {0x04, 0x54, 0x53, "🍎 Apple", "Apple"}, {0x08, 0x66, 0x98, "🍎 Apple", "Apple"},
    {0x0C, 0x4D, 0xE9, "🍎 Apple", "Apple"}, {0x0C, 0xC5, 0x6C, "🍎 Apple", "Apple"},
    {0x0C, 0x74, 0xC2, "🍎 Apple", "Apple"}, {0x0C, 0xBC, 0x9F, "🍎 Apple", "Apple"},
    {0x0C, 0xD7, 0x46, "🍎 Apple", "Apple"}, {0x10, 0x94, 0xBB, "🍎 Apple", "Apple"},
    {0x14, 0x7D, 0xDA, "🍎 Apple", "Apple"}, {0x18, 0xAF, 0x61, "🍎 Apple", "Apple"},
    {0x20, 0xA2, 0xE4, "🍎 Apple", "Apple"}, {0x24, 0xA0, 0x74, "🍎 Apple", "Apple"},
    {0x28, 0xCF, 0xE9, "🍎 Apple", "Apple"}, {0x38, 0xCA, 0x84, "🍎 Apple", "Apple"},
    {0x3C, 0x07, 0x54, "🍎 Apple", "Apple"}, {0x40, 0xA6, 0xD9, "🍎 Apple", "Apple"},
    {0x44, 0xFB, 0x42, "🍎 Apple", "Apple"}, {0x48, 0x60, 0x5F, "🍎 Apple", "Apple"},
    {0x4C, 0x32, 0x75, "🍎 Apple", "Apple"}, {0x50, 0xBC, 0x96, "🍎 Apple", "Apple"},
    {0x54, 0x26, 0x96, "🍎 Apple", "Apple"}, {0x60, 0x03, 0x08, "🍎 Apple", "Apple"},
    {0x64, 0xB0, 0xA6, "🍎 Apple", "Apple"}, {0x68, 0x5B, 0x35, "🍎 Apple", "Apple"},
    {0x70, 0xEC, 0xE4, "🍎 Apple", "Apple"}, {0x78, 0x7B, 0x8A, "🍎 Apple", "Apple"},
    {0x7C, 0x6D, 0x62, "🍎 Apple", "Apple"}, {0x80, 0xE6, 0x50, "🍎 Apple", "Apple"},
    {0x84, 0x38, 0x35, "🍎 Apple", "Apple"}, {0x88, 0x66, 0x5A, "🍎 Apple", "Apple"},
    {0x90, 0xFD, 0x61, "🍎 Apple", "Apple"}, {0x94, 0xE9, 0x79, "🍎 Apple", "Apple"},
    {0x98, 0x10, 0xE8, "🍎 Apple", "Apple"}, {0xA4, 0x83, 0xE7, "🍎 Apple", "Apple"},
    {0xA8, 0x66, 0x7F, "🍎 Apple", "Apple"}, {0xAC, 0xBC, 0x32, "🍎 Apple", "Apple"},
    {0xB0, 0x34, 0x95, "🍎 Apple", "Apple"}, {0xB4, 0x18, 0xD1, "🍎 Apple", "Apple"},
    {0xBC, 0x54, 0x36, "🍎 Apple", "Apple"}, {0xC8, 0x69, 0xCD, "🍎 Apple", "Apple"},
    {0xD0, 0x81, 0x7A, "🍎 Apple", "Apple"}, {0xD8, 0x96, 0x95, "🍎 Apple", "Apple"},
    {0xDC, 0x2B, 0x61, "🍎 Apple", "Apple"}, {0xE0, 0x66, 0x78, "🍎 Apple", "Apple"},
    {0xE4, 0xCE, 0x8F, "🍎 Apple", "Apple"}, {0xF0, 0x18, 0x98, "🍎 Apple", "Apple"},
    {0xF4, 0x5C, 0x89, "🍎 Apple", "Apple"}, {0xF4, 0xF1, 0x5A, "🍎 Apple", "Apple"},
    {0xFC, 0xFC, 0x48, "🍎 Apple", "Apple"},

    // ==================== Intel (PC / Laptop Wi-Fi 6/6E/7 / NUC) ====================
    {0xAC, 0x82, 0x47, "💻 Intel PC", "Intel PC"}, {0x00, 0x02, 0xB3, "💻 Intel PC", "Intel PC"},
    {0x00, 0x03, 0x47, "💻 Intel PC", "Intel PC"}, {0x00, 0x04, 0x23, "💻 Intel PC", "Intel PC"},
    {0x00, 0x0C, 0xF1, "💻 Intel PC", "Intel PC"}, {0x00, 0x0E, 0x0C, "💻 Intel PC", "Intel PC"},
    {0x00, 0x11, 0x11, "💻 Intel PC", "Intel PC"}, {0x00, 0x13, 0x02, "💻 Intel PC", "Intel PC"},
    {0x00, 0x13, 0x20, "💻 Intel PC", "Intel PC"}, {0x00, 0x13, 0xE8, "💻 Intel PC", "Intel PC"},
    {0x00, 0x15, 0x00, "💻 Intel PC", "Intel PC"}, {0x00, 0x15, 0x17, "💻 Intel PC", "Intel PC"},
    {0x00, 0x16, 0x76, "💻 Intel PC", "Intel PC"}, {0x00, 0x19, 0xD1, "💻 Intel PC", "Intel PC"},
    {0x00, 0x1B, 0x21, "💻 Intel PC", "Intel PC"}, {0x00, 0x1C, 0xC0, "💻 Intel PC", "Intel PC"},
    {0x00, 0x1D, 0xE0, "💻 Intel PC", "Intel PC"}, {0x00, 0x1E, 0x67, "💻 Intel PC", "Intel PC"},
    {0x00, 0x21, 0x5C, "💻 Intel PC", "Intel PC"}, {0x00, 0x21, 0x6A, "💻 Intel PC", "Intel PC"},
    {0x00, 0x22, 0xFB, "💻 Intel PC", "Intel PC"}, {0x00, 0x23, 0x14, "💻 Intel PC", "Intel PC"},
    {0x00, 0x24, 0xD7, "💻 Intel PC", "Intel PC"}, {0x00, 0x27, 0x0E, "💻 Intel PC", "Intel PC"},
    {0x00, 0x28, 0xF8, "💻 Intel PC", "Intel PC"}, {0x08, 0x11, 0x96, "💻 Intel PC", "Intel PC"},
    {0x24, 0x77, 0x03, "💻 Intel PC", "Intel PC"}, {0x34, 0x13, 0xE8, "💻 Intel PC", "Intel PC"},
    {0x38, 0xDE, 0xAD, "💻 Intel PC", "Intel PC"}, {0x48, 0x51, 0xB7, "💻 Intel PC", "Intel PC"},
    {0x4C, 0x79, 0xBA, "💻 Intel PC", "Intel PC"}, {0x5C, 0x87, 0x9C, "💻 Intel PC", "Intel PC"},
    {0x68, 0x05, 0xCA, "💻 Intel PC", "Intel PC"}, {0x70, 0x1C, 0xE8, "💻 Intel PC", "Intel PC"},
    {0x80, 0x86, 0xF2, "💻 Intel PC", "Intel PC"}, {0x84, 0xA9, 0x3E, "💻 Intel PC", "Intel PC"},
    {0x8C, 0x85, 0x90, "💻 Intel PC", "Intel PC"}, {0xA0, 0x36, 0xBC, "💻 Intel PC", "Intel PC"},
    {0xA4, 0x4C, 0xC8, "💻 Intel PC", "Intel PC"}, {0xA8, 0x70, 0xA5, "💻 Intel PC", "Intel PC"},
    {0xB8, 0x08, 0xCF, "💻 Intel PC", "Intel PC"}, {0xC8, 0x5B, 0x76, "💻 Intel PC", "Intel PC"},
    {0xD4, 0x3B, 0x04, "💻 Intel PC", "Intel PC"}, {0xD4, 0x5D, 0x64, "💻 Intel PC", "Intel PC"},
    {0xE0, 0x94, 0x67, "💻 Intel PC", "Intel PC"}, {0xE8, 0x2A, 0x44, "💻 Intel PC", "Intel PC"},
    {0xF4, 0x46, 0xE6, "💻 Intel PC", "Intel PC"}, {0xF8, 0x59, 0x71, "💻 Intel PC", "Intel PC"},

    // ==================== Samsung (Galaxy S/A/Z, Galaxy Tab, Smart TV) ====================
    {0x00, 0x07, 0xAB, "📱 三星 Samsung", "Samsung"}, {0x00, 0x12, 0x47, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x15, 0xB9, "📱 三星 Samsung", "Samsung"}, {0x00, 0x16, 0x32, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x16, 0x6C, "📱 三星 Samsung", "Samsung"}, {0x00, 0x17, 0xC9, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x18, 0xAF, "📱 三星 Samsung", "Samsung"}, {0x00, 0x1A, 0x8A, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x1D, 0x25, "📱 三星 Samsung", "Samsung"}, {0x00, 0x1E, 0xE2, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x21, 0x19, "📱 三星 Samsung", "Samsung"}, {0x00, 0x21, 0xD1, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x23, 0x39, "📱 三星 Samsung", "Samsung"}, {0x00, 0x24, 0x54, "📱 三星 Samsung", "Samsung"},
    {0x00, 0x26, 0x37, "📱 三星 Samsung", "Samsung"}, {0x08, 0x08, 0xC2, "📱 三星 Samsung", "Samsung"},
    {0x08, 0xD4, 0x6A, "📱 三星 Samsung", "Samsung"}, {0x10, 0xD5, 0x42, "📱 三星 Samsung", "Samsung"},
    {0x14, 0x49, 0xE0, "📱 三星 Samsung", "Samsung"}, {0x18, 0x22, 0x7E, "📱 三星 Samsung", "Samsung"},
    {0x24, 0x4B, 0x81, "📱 三星 Samsung", "Samsung"}, {0x28, 0x27, 0xBF, "📱 三星 Samsung", "Samsung"},
    {0x30, 0xCD, 0xA7, "📱 三星 Samsung", "Samsung"}, {0x34, 0x82, 0xC5, "📱 三星 Samsung", "Samsung"},
    {0x40, 0x40, 0xA7, "📱 三星 Samsung", "Samsung"}, {0x4C, 0x66, 0x41, "📱 三星 Samsung", "Samsung"},
    {0x50, 0x77, 0x05, "📱 三星 Samsung", "Samsung"}, {0x54, 0x88, 0x0E, "📱 三星 Samsung", "Samsung"},
    {0x58, 0xC3, 0x8B, "📱 三星 Samsung", "Samsung"}, {0x64, 0xB3, 0x10, "📱 三星 Samsung", "Samsung"},
    {0x70, 0x2C, 0x1F, "📱 三星 Samsung", "Samsung"}, {0x78, 0x47, 0x1D, "📱 三星 Samsung", "Samsung"},
    {0x84, 0x25, 0x19, "📱 三星 Samsung", "Samsung"}, {0x88, 0x36, 0x6C, "📱 三星 Samsung", "Samsung"},
    {0x90, 0x18, 0x7C, "📱 三星 Samsung", "Samsung"}, {0x94, 0x01, 0xC2, "📱 三星 Samsung", "Samsung"},
    {0xA0, 0x82, 0x1F, "📱 三星 Samsung", "Samsung"}, {0xA4, 0xC4, 0x94, "📱 三星 Samsung", "Samsung"},
    {0xB0, 0xEC, 0x71, "📱 三星 Samsung", "Samsung"}, {0xBC, 0x85, 0x56, "📱 三星 Samsung", "Samsung"},
    {0xC4, 0x73, 0x1E, "📱 三星 Samsung", "Samsung"}, {0xCC, 0x07, 0xAB, "📱 三星 Samsung", "Samsung"},
    {0xD0, 0x03, 0xDF, "📱 三星 Samsung", "Samsung"}, {0xE0, 0xDB, 0x55, "📱 三星 Samsung", "Samsung"},
    {0xE8, 0x50, 0x8B, "📱 三星 Samsung", "Samsung"}, {0xF8, 0x77, 0xB8, "📱 三星 Samsung", "Samsung"},

    // ==================== ASUS (華碩 ROG / ZenBook / TUF / 主機板 / 路由器) ====================
    {0x00, 0x0C, 0x6E, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x11, 0xD8, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x13, 0xD4, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x15, 0xF2, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x17, 0x31, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x18, 0xF3, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x1A, 0x92, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x1B, 0xFC, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x1D, 0x60, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x1E, 0x8C, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x22, 0x15, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x23, 0x54, "💻 華碩 ASUS", "ASUS"},
    {0x00, 0x24, 0x8C, "💻 華碩 ASUS", "ASUS"}, {0x00, 0x26, 0x18, "💻 華碩 ASUS", "ASUS"},
    {0x04, 0xD4, 0xC4, "💻 華碩 ASUS", "ASUS"}, {0x08, 0x60, 0x6E, "💻 華碩 ASUS", "ASUS"},
    {0x08, 0x62, 0x66, "💻 華碩 ASUS", "ASUS"}, {0x10, 0x7B, 0x44, "💻 華碩 ASUS", "ASUS"},
    {0x18, 0x31, 0xBF, "💻 華碩 ASUS", "ASUS"}, {0x2C, 0xFD, 0xA1, "💻 華碩 ASUS", "ASUS"},
    {0x38, 0xD5, 0x47, "💻 華碩 ASUS", "ASUS"}, {0x40, 0x16, 0x7E, "💻 華碩 ASUS", "ASUS"},
    {0x50, 0x46, 0x5D, "💻 華碩 ASUS", "ASUS"}, {0x54, 0xA0, 0x50, "💻 華碩 ASUS", "ASUS"},
    {0x60, 0x45, 0xCB, "💻 華碩 ASUS", "ASUS"}, {0x70, 0x4D, 0x7B, "💻 華碩 ASUS", "ASUS"},
    {0x90, 0xE6, 0xBA, "💻 華碩 ASUS", "ASUS"}, {0xAC, 0x9E, 0x17, "💻 華碩 ASUS", "ASUS"},
    {0xBC, 0xEE, 0x7B, "💻 華碩 ASUS", "ASUS"}, {0xD8, 0x50, 0xE6, "💻 華碩 ASUS", "ASUS"},
    {0xF0, 0x2F, 0x74, "💻 華碩 ASUS", "ASUS"}, {0xF8, 0x32, 0xE4, "💻 華碩 ASUS", "ASUS"},

    // ==================== Acer (宏碁 Predator / Swift / Aspire) ====================
    {0x00, 0x01, 0x24, "💻 宏碁 Acer", "Acer"}, {0x00, 0x03, 0x0D, "💻 宏碁 Acer", "Acer"},
    {0x00, 0x16, 0x76, "💻 宏碁 Acer", "Acer"}, {0x00, 0x60, 0x67, "💻 宏碁 Acer", "Acer"},
    {0x00, 0xA0, 0x60, "💻 宏碁 Acer", "Acer"}, {0x0C, 0x84, 0xDC, "💻 宏碁 Acer", "Acer"},
    {0x18, 0x06, 0xFF, "💻 宏碁 Acer", "Acer"}, {0x38, 0x63, 0xBB, "💻 宏碁 Acer", "Acer"},
    {0xC0, 0x98, 0x79, "💻 宏碁 Acer", "Acer"}, {0xD8, 0x9D, 0x67, "💻 宏碁 Acer", "Acer"},

    // ==================== MSI (微星 Gaming 筆電 / 主機板) ====================
    {0x00, 0x16, 0x17, "💻 微星 MSI", "MSI"}, {0x00, 0x19, 0xDB, "💻 微星 MSI", "MSI"},
    {0x00, 0x21, 0x85, "💻 微星 MSI", "MSI"}, {0x00, 0x24, 0x21, "💻 微星 MSI", "MSI"},
    {0x00, 0x26, 0x5A, "💻 微星 MSI", "MSI"}, {0x00, 0x30, 0x18, "💻 微星 MSI", "MSI"},
    {0x00, 0x40, 0xF4, "💻 微星 MSI", "MSI"}, {0x2C, 0xF0, 0x5D, "💻 微星 MSI", "MSI"},
    {0x44, 0x8A, 0x5B, "💻 微星 MSI", "MSI"}, {0xD8, 0x5E, 0xD3, "💻 微星 MSI", "MSI"},

    // ==================== GIGABYTE (技嘉 AORUS / 主機板 / 筆電) ====================
    {0x00, 0x0D, 0x56, "💻 技嘉 GIGABYTE", "Gigabyte"}, {0x00, 0x16, 0xE6, "💻 技嘉 GIGABYTE", "Gigabyte"},
    {0x00, 0x18, 0xF3, "💻 技嘉 GIGABYTE", "Gigabyte"}, {0x00, 0x1D, 0x7D, "💻 技嘉 GIGABYTE", "Gigabyte"},
    {0x00, 0x24, 0x1D, "💻 技嘉 GIGABYTE", "Gigabyte"}, {0x1C, 0x1B, 0x0D, "💻 技嘉 GIGABYTE", "Gigabyte"},
    {0x40, 0x8D, 0x5C, "💻 技嘉 GIGABYTE", "Gigabyte"}, {0x6C, 0x4B, 0x90, "💻 技嘉 GIGABYTE", "Gigabyte"},
    {0xE0, 0xD5, 0x5E, "💻 技嘉 GIGABYTE", "Gigabyte"}, {0xFC, 0xAA, 0x14, "💻 技嘉 GIGABYTE", "Gigabyte"},

    // ==================== Sony (PlayStation 5 / PS4 / Bravia / Xperia) ====================
    {0x00, 0x04, 0x1F, "🎮 Sony PlayStation", "Sony PS"}, {0x00, 0x13, 0x15, "🎮 Sony PlayStation", "Sony PS"},
    {0x00, 0x15, 0xC1, "🎮 Sony PlayStation", "Sony PS"}, {0x00, 0x19, 0xC5, "🎮 Sony PlayStation", "Sony PS"},
    {0x00, 0x1A, 0x80, "🎮 Sony PlayStation", "Sony PS"}, {0x00, 0x1D, 0xBA, "🎮 Sony PlayStation", "Sony PS"},
    {0x00, 0x1F, 0xA7, "🎮 Sony PlayStation", "Sony PS"}, {0x00, 0x24, 0x8D, "🎮 Sony PlayStation", "Sony PS"},
    {0x0C, 0xFE, 0x45, "🎮 Sony PlayStation", "Sony PS"}, {0x28, 0x0D, 0xFC, "🎮 Sony PlayStation", "Sony PS"},
    {0x70, 0x9E, 0x29, "🎮 Sony PlayStation", "Sony PS"}, {0x78, 0xC8, 0x81, "🎮 Sony PlayStation", "Sony PS"},
    {0xA8, 0xE3, 0xEE, "🎮 Sony PlayStation", "Sony PS"}, {0xBC, 0x60, 0xA7, "🎮 Sony PlayStation", "Sony PS"},
    {0xF8, 0x46, 0x1C, "🎮 Sony PlayStation", "Sony PS"}, {0xFC, 0x0F, 0xE6, "🎮 Sony PlayStation", "Sony PS"},

    // ==================== Nintendo (任天堂 Switch / Switch OLED) ====================
    {0x00, 0x09, 0xBF, "🎮 任天堂 Nintendo", "Nintendo"}, {0x00, 0x17, 0xAB, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x00, 0x19, 0x1D, "🎮 任天堂 Nintendo", "Nintendo"}, {0x00, 0x1B, 0x7A, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x00, 0x1F, 0x32, "🎮 任天堂 Nintendo", "Nintendo"}, {0x00, 0x21, 0x47, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x00, 0x22, 0xD7, "🎮 任天堂 Nintendo", "Nintendo"}, {0x00, 0x24, 0x1E, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x00, 0x24, 0x44, "🎮 任天堂 Nintendo", "Nintendo"}, {0x00, 0x25, 0xA0, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x00, 0x26, 0x59, "🎮 任天堂 Nintendo", "Nintendo"}, {0x04, 0x03, 0xD6, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x20, 0x47, 0xDA, "🎮 任天堂 Nintendo", "Nintendo"}, {0x34, 0xAF, 0x2C, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x70, 0x48, 0x0F, "🎮 任天堂 Nintendo", "Nintendo"}, {0x7C, 0xBB, 0x8A, "🎮 任天堂 Nintendo", "Nintendo"},
    {0x98, 0xB6, 0xE9, "🎮 任天堂 Nintendo", "Nintendo"}, {0xA4, 0x5C, 0x27, "🎮 任天堂 Nintendo", "Nintendo"},
    {0xD8, 0x6B, 0xF7, "🎮 任天堂 Nintendo", "Nintendo"}, {0xE0, 0xE7, 0x51, "🎮 任天堂 Nintendo", "Nintendo"},

    // ==================== Microsoft (Windows / Surface / Xbox Series) ====================
    {0x00, 0x03, 0xFF, "🪟 微軟 Microsoft", "Microsoft"}, {0x00, 0x0D, 0x3A, "🪟 微軟 Microsoft", "Microsoft"},
    {0x00, 0x12, 0x5A, "🪟 微軟 Microsoft", "Microsoft"}, {0x00, 0x15, 0x5D, "🪟 微軟 Microsoft", "Microsoft"},
    {0x00, 0x17, 0xFA, "🪟 微軟 Microsoft", "Microsoft"}, {0x00, 0x1D, 0xD8, "🪟 微軟 Microsoft", "Microsoft"},
    {0x00, 0x22, 0x48, "🪟 微軟 Microsoft", "Microsoft"}, {0x00, 0x25, 0xAE, "🪟 微軟 Microsoft", "Microsoft"},
    {0x00, 0x50, 0xF2, "🪟 微軟 Microsoft", "Microsoft"}, {0x28, 0x18, 0x78, "🪟 微軟 Microsoft", "Microsoft"},
    {0x30, 0x59, 0xB7, "🪟 微軟 Microsoft", "Microsoft"}, {0x60, 0x45, 0xBD, "🪟 微軟 Microsoft", "Microsoft"},
    {0x7C, 0x1E, 0x52, "🪟 微軟 Microsoft", "Microsoft"}, {0x98, 0x5F, 0xD3, "🪟 微軟 Microsoft", "Microsoft"},
    {0xA0, 0x85, 0xFC, "🪟 微軟 Microsoft", "Microsoft"}, {0xC0, 0x33, 0x5E, "🪟 微軟 Microsoft", "Microsoft"},
    {0xDC, 0x98, 0x40, "🪟 微軟 Microsoft", "Microsoft"},

    // ==================== Google (Pixel / Nest / Chromecast) ====================
    {0x00, 0x1A, 0x11, "📱 Google", "Google"}, {0x20, 0xDF, 0xB9, "📱 Google", "Google"},
    {0x3C, 0x5A, 0xB4, "📱 Google", "Google"}, {0x48, 0xD7, 0x05, "📱 Google", "Google"},
    {0x54, 0x60, 0x09, "📱 Google", "Google"}, {0x6C, 0xAD, 0xF8, "📱 Google", "Google"},
    {0x94, 0xEB, 0xCD, "📱 Google", "Google"}, {0xA4, 0x77, 0x33, "📱 Google", "Google"},
    {0xD8, 0x6C, 0x63, "📱 Google", "Google"}, {0xE4, 0xF0, 0x42, "📱 Google", "Google"},
    {0xF8, 0x8F, 0xC2, "📱 Google", "Google"},

    // ==================== Xiaomi (小米 / 紅米 Redmi / POCO) ====================
    {0x00, 0x9E, 0xC8, "📱 小米 Xiaomi", "Xiaomi"}, {0x0C, 0x98, 0x38, "📱 小米 Xiaomi", "Xiaomi"},
    {0x14, 0xF6, 0x5A, "📱 小米 Xiaomi", "Xiaomi"}, {0x18, 0x59, 0x36, "📱 小米 Xiaomi", "Xiaomi"},
    {0x28, 0x6C, 0x07, "📱 小米 Xiaomi", "Xiaomi"}, {0x34, 0x80, 0xB3, "📱 小米 Xiaomi", "Xiaomi"},
    {0x50, 0x64, 0x2B, "📱 小米 Xiaomi", "Xiaomi"}, {0x64, 0x09, 0x80, "📱 小米 Xiaomi", "Xiaomi"},
    {0x74, 0x51, 0xBA, "📱 小米 Xiaomi", "Xiaomi"}, {0x7C, 0x1D, 0xD9, "📱 小米 Xiaomi", "Xiaomi"},
    {0x8C, 0xBE, 0xBE, "📱 小米 Xiaomi", "Xiaomi"}, {0xAC, 0xC1, 0xEE, "📱 小米 Xiaomi", "Xiaomi"},
    {0xD4, 0x97, 0x0B, "📱 小米 Xiaomi", "Xiaomi"}, {0xE4, 0xAA, 0xEC, "📱 小米 Xiaomi", "Xiaomi"},
    {0xF8, 0xA4, 0x5F, "📱 小米 Xiaomi", "Xiaomi"},

    // ==================== Huawei / Honor (華為 / 榮耀) ====================
    {0x00, 0x18, 0x82, "📱 華為 Huawei", "Huawei"}, {0x00, 0x1E, 0x10, "📱 華為 Huawei", "Huawei"},
    {0x00, 0x25, 0x68, "📱 華為 Huawei", "Huawei"}, {0x00, 0x25, 0x9E, "📱 華為 Huawei", "Huawei"},
    {0x00, 0x46, 0x4B, "📱 華為 Huawei", "Huawei"}, {0x04, 0x25, 0x7B, "📱 華為 Huawei", "Huawei"},
    {0x04, 0x79, 0x70, "📱 華為 Huawei", "Huawei"}, {0x04, 0xC0, 0x6F, "📱 華為 Huawei", "Huawei"},
    {0x08, 0x19, 0xA6, "📱 華為 Huawei", "Huawei"}, {0x0C, 0x96, 0xBF, "📱 華為 Huawei", "Huawei"},
    {0x10, 0x47, 0x80, "📱 華為 Huawei", "Huawei"}, {0x20, 0x08, 0x89, "📱 華為 Huawei", "Huawei"},
    {0x48, 0x46, 0xFB, "📱 華為 Huawei", "Huawei"}, {0x54, 0x89, 0x98, "📱 華為 Huawei", "Huawei"},
    {0x70, 0x54, 0xF5, "📱 華為 Huawei", "Huawei"}, {0x88, 0x53, 0xD4, "📱 華為 Huawei", "Huawei"},
    {0xAC, 0xE2, 0x15, "📱 華為 Huawei", "Huawei"}, {0xCC, 0x96, 0xA0, "📱 華為 Huawei", "Huawei"},
    {0xDC, 0xD2, 0xFC, "📱 華為 Huawei", "Huawei"},

    // ==================== OPPO / OnePlus / Realme ====================
    {0x10, 0x2A, 0xB3, "📱 OPPO/Realme", "OPPO"}, {0x14, 0xAB, 0xC5, "📱 OPPO/Realme", "OPPO"},
    {0x2C, 0x59, 0x8A, "📱 OPPO/Realme", "OPPO"}, {0x30, 0x0C, 0x23, "📱 OPPO/Realme", "OPPO"},
    {0x34, 0xCE, 0x00, "📱 OPPO/Realme", "OPPO"}, {0x48, 0x13, 0x7E, "📱 OPPO/Realme", "OPPO"},
    {0x50, 0x01, 0xD9, "📱 OPPO/Realme", "OPPO"}, {0x70, 0x8A, 0x09, "📱 OPPO/Realme", "OPPO"},
    {0x84, 0xDB, 0xAC, "📱 OPPO/Realme", "OPPO"}, {0x90, 0x78, 0x41, "📱 OPPO/Realme", "OPPO"},
    {0xA8, 0x9C, 0xED, "📱 OPPO/Realme", "OPPO"}, {0xC8, 0xF2, 0x30, "📱 OPPO/Realme", "OPPO"},
    {0xE4, 0x46, 0xDA, "📱 OPPO/Realme", "OPPO"},

    // ==================== vivo / iQOO ====================
    {0x04, 0xCF, 0x8C, "📱 vivo", "vivo"}, {0x10, 0x2C, 0x6B, "📱 vivo", "vivo"},
    {0x28, 0x5A, 0xEB, "📱 vivo", "vivo"}, {0x50, 0x33, 0x8B, "📱 vivo", "vivo"},
    {0x68, 0x3E, 0x34, "📱 vivo", "vivo"}, {0x74, 0xAC, 0x5F, "📱 vivo", "vivo"},
    {0x88, 0xC9, 0xD0, "📱 vivo", "vivo"}, {0xB0, 0x7D, 0x62, "📱 vivo", "vivo"},
    {0xC4, 0xB3, 0x01, "📱 vivo", "vivo"}, {0xE8, 0x4E, 0x06, "📱 vivo", "vivo"},

    // ==================== TP-Link (路由器 / Deco / Tapo 攝影機) ====================
    {0x00, 0x27, 0x19, "📡 TP-Link", "TP-Link"}, {0x14, 0xCC, 0x20, "📡 TP-Link", "TP-Link"},
    {0x1C, 0x3B, 0xF3, "📡 TP-Link", "TP-Link"}, {0x30, 0xB5, 0xC2, "📡 TP-Link", "TP-Link"},
    {0x50, 0xC7, 0xBF, "📡 TP-Link", "TP-Link"}, {0x54, 0xAF, 0x97, "📡 TP-Link", "TP-Link"},
    {0x54, 0xC8, 0x0F, "📡 TP-Link", "TP-Link"}, {0x60, 0xA4, 0x4C, "📡 TP-Link", "TP-Link"},
    {0x70, 0x4F, 0x57, "📡 TP-Link", "TP-Link"}, {0x98, 0xDA, 0xC4, "📡 TP-Link", "TP-Link"},
    {0xB0, 0x95, 0x75, "📡 TP-Link", "TP-Link"}, {0xC0, 0x06, 0xC3, "📡 TP-Link", "TP-Link"},
    {0xC0, 0xA5, 0xDD, "📡 TP-Link", "TP-Link"}, {0xD8, 0x07, 0xB6, "📡 TP-Link", "TP-Link"},
    {0xE8, 0x48, 0xB8, "📡 TP-Link", "TP-Link"}, {0xEC, 0x08, 0x6B, "📡 TP-Link", "TP-Link"},
    {0xF4, 0xF2, 0x6D, "📡 TP-Link", "TP-Link"},

    // ==================== D-Link (友訊科技) ====================
    {0x00, 0x05, 0x5D, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x0D, 0x88, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x13, 0x46, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x15, 0xE9, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x17, 0x9A, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x19, 0x5B, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x1B, 0x11, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x1C, 0xF0, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x1E, 0x58, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x21, 0x91, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x22, 0xB0, "📡 友訊 D-Link", "D-Link"}, {0x00, 0x24, 0x01, "📡 友訊 D-Link", "D-Link"},
    {0x00, 0x26, 0x5A, "📡 友訊 D-Link", "D-Link"}, {0x14, 0xD6, 0x4D, "📡 友訊 D-Link", "D-Link"},
    {0x28, 0x10, 0x7B, "📡 友訊 D-Link", "D-Link"}, {0xB0, 0xC5, 0x54, "📡 友訊 D-Link", "D-Link"},
    {0xC8, 0xD3, 0xA3, "📡 友訊 D-Link", "D-Link"},

    // ==================== ZyXEL (合勤科技) ====================
    {0x00, 0x02, 0xCF, "📡 合勤 ZyXEL", "ZyXEL"}, {0x00, 0x13, 0x49, "📡 合勤 ZyXEL", "ZyXEL"},
    {0x00, 0x19, 0xCB, "📡 合勤 ZyXEL", "ZyXEL"}, {0x00, 0x1B, 0x32, "📡 合勤 ZyXEL", "ZyXEL"},
    {0x00, 0x23, 0xF8, "📡 合勤 ZyXEL", "ZyXEL"}, {0x00, 0xA0, 0xC5, "📡 合勤 ZyXEL", "ZyXEL"},
    {0x40, 0x4A, 0x03, "📡 合勤 ZyXEL", "ZyXEL"}, {0x50, 0x67, 0xF0, "📡 合勤 ZyXEL", "ZyXEL"},
    {0xCC, 0x5D, 0x4E, "📡 合勤 ZyXEL", "ZyXEL"},

    // ==================== Realtek (瑞昱 網卡晶片) ====================
    {0x00, 0x07, 0x32, "💻 瑞昱 Realtek", "Realtek"}, {0x00, 0xE0, 0x4C, "💻 瑞昱 Realtek", "Realtek"},
    {0x20, 0x76, 0x93, "💻 瑞昱 Realtek", "Realtek"}, {0x52, 0x54, 0x4C, "💻 瑞昱 Realtek", "Realtek"},
    {0x54, 0xE6, 0xFC, "💻 瑞昱 Realtek", "Realtek"}, {0x70, 0x85, 0xC2, "💻 瑞昱 Realtek", "Realtek"},
    {0x88, 0xD7, 0xF6, "💻 瑞昱 Realtek", "Realtek"},

    // ==================== MediaTek (聯發科技) ====================
    {0x00, 0x0C, 0xE7, "📶 聯發科 MTK", "MediaTek"}, {0x08, 0xBE, 0xAC, "📶 聯發科 MTK", "MediaTek"},
    {0x14, 0x91, 0x82, "📶 聯發科 MTK", "MediaTek"}, {0x1C, 0xBF, 0xCE, "📶 聯發科 MTK", "MediaTek"},
    {0x3C, 0xCD, 0x5D, "📶 聯發科 MTK", "MediaTek"}, {0x70, 0x66, 0x55, "📶 聯發科 MTK", "MediaTek"},
    {0x78, 0x65, 0x59, "📶 聯發科 MTK", "MediaTek"}, {0x88, 0x9F, 0xFA, "📶 聯發科 MTK", "MediaTek"},
    {0x98, 0x93, 0xCC, "📶 聯發科 MTK", "MediaTek"},

    // ==================== Raspberry Pi (樹莓派) ====================
    {0x28, 0xCD, 0xC1, "🍓 樹莓派 RPi", "Raspberry Pi"}, {0xB8, 0x27, 0xEB, "🍓 樹莓派 RPi", "Raspberry Pi"},
    {0xDC, 0xA6, 0x32, "🍓 樹莓派 RPi", "Raspberry Pi"}, {0xE4, 0x5F, 0x01, "🍓 樹莓派 RPi", "Raspberry Pi"},

    // ==================== Espressif (樂鑫 ESP32 / ESP8266 IoT) ====================
    {0x24, 0x0A, 0xC4, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x24, 0x62, 0xAB, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x24, 0x6F, 0x28, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x24, 0xB2, 0xDE, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x30, 0xAE, 0xA4, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x3C, 0x61, 0x05, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x3C, 0x71, 0xBF, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x48, 0x27, 0xE2, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x4C, 0x11, 0xAE, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x4C, 0x75, 0x25, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x54, 0x43, 0xB2, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x5C, 0xCF, 0x7F, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x68, 0xC6, 0x3A, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x70, 0x03, 0x9F, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x78, 0x21, 0x84, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x84, 0xCC, 0xA8, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x84, 0xF7, 0x03, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x8C, 0xAA, 0xB5, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x94, 0xB5, 0x55, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0x94, 0xB9, 0x7E, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0x98, 0xF4, 0xAB, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xA0, 0x20, 0xA6, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xA0, 0x76, 0x4E, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xA4, 0xCF, 0x12, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xA4, 0xE5, 0x7C, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xAC, 0x67, 0xB2, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xB4, 0xE6, 0x2D, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xBC, 0xFF, 0x4D, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xC4, 0x4F, 0x33, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xC8, 0x2E, 0x18, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xCC, 0x50, 0xE3, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xD8, 0xA0, 0x1D, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xDC, 0x4F, 0x22, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xDC, 0x54, 0x75, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},
    {0xEC, 0x62, 0x60, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"}, {0xEC, 0xDA, 0x3B, "⚡ 樂鑫 ESP32 IoT", "ESP32 IoT"},

    // ==================== Amazon (Echo / Fire TV / Kindle) ====================
    {0x00, 0xFC, 0x8B, "📦 亞馬遜 Amazon", "Amazon"}, {0x18, 0x74, 0x2E, "📦 亞馬遜 Amazon", "Amazon"},
    {0x34, 0xD2, 0x70, "📦 亞馬遜 Amazon", "Amazon"}, {0x38, 0xF7, 0x3D, "📦 亞馬遜 Amazon", "Amazon"},
    {0x40, 0xB4, 0xCD, "📦 亞馬遜 Amazon", "Amazon"}, {0x44, 0x65, 0x0D, "📦 亞馬遜 Amazon", "Amazon"},
    {0x50, 0xF5, 0xDA, "📦 亞馬遜 Amazon", "Amazon"}, {0x68, 0x37, 0xE9, "📦 亞馬遜 Amazon", "Amazon"},
    {0x68, 0x54, 0x5A, "📦 亞馬遜 Amazon", "Amazon"}, {0x74, 0xC2, 0x46, "📦 亞馬遜 Amazon", "Amazon"},
    {0x84, 0xD6, 0xD0, "📦 亞馬遜 Amazon", "Amazon"}, {0xAC, 0x63, 0xBE, "📦 亞馬遜 Amazon", "Amazon"},
    {0xFC, 0xA6, 0x67, "📦 亞馬遜 Amazon", "Amazon"},

    // ==================== LG Electronics (智慧電視 / 智慧家電) ====================
    {0x00, 0x05, 0xC9, "📺 LG 電視/家電", "LG TV"}, {0x00, 0x1E, 0x75, "📺 LG 電視/家電", "LG TV"},
    {0x00, 0x1F, 0x6B, "📺 LG 電視/家電", "LG TV"}, {0x00, 0x24, 0x83, "📺 LG 電視/家電", "LG TV"},
    {0x10, 0x68, 0x3F, "📺 LG 電視/家電", "LG TV"}, {0x18, 0x26, 0x66, "📺 LG 電視/家電", "LG TV"},
    {0x34, 0xFC, 0xEF, "📺 LG 電視/家電", "LG TV"}, {0x58, 0xA2, 0xB5, "📺 LG 電視/家電", "LG TV"},
    {0x64, 0x99, 0x5D, "📺 LG 電視/家電", "LG TV"}, {0x74, 0x40, 0xBE, "📺 LG 電視/家電", "LG TV"},
    {0xA8, 0x23, 0xFE, "📺 LG 電視/家電", "LG TV"}, {0xC8, 0x08, 0xE9, "📺 LG 電視/家電", "LG TV"},
    {0xCC, 0x2D, 0x83, "📺 LG 電視/家電", "LG TV"},

    // ==================== NAS Storage (Synology 群暉 / QNAP 威聯通) ====================
    {0x00, 0x11, 0x32, "💾 群暉 Synology", "Synology"}, {0x00, 0x23, 0x45, "💾 群暉 Synology", "Synology"},
    {0x90, 0x09, 0xD0, "💾 群暉 Synology", "Synology"},
    {0x00, 0x08, 0x9B, "💾 威聯通 QNAP", "QNAP"}, {0x24, 0x5E, 0xBE, "💾 威聯通 QNAP", "QNAP"}
};

static const size_t VENDOR_COUNT = sizeof(VENDOR_TABLE) / sizeof(VENDOR_TABLE[0]);

// ==================== Custom Device Alias Store ====================
struct DevAlias {
    uint32_t ip;
    char name[36];
};

static const int MAX_ALIASES = 64;
static DevAlias aliasList[MAX_ALIASES];
static int numAliases = 0;

static void saveAliases() {
    File f = LittleFS.open("/aliases.txt", "w");
    if (!f) return;
    for (int i = 0; i < numAliases; i++) {
        IPAddress ip(aliasList[i].ip);
        f.printf("%s,%s\n", ip.toString().c_str(), aliasList[i].name);
    }
    f.close();
}

void initDeviceManager() {
    numAliases = 0;
    File f = LittleFS.open("/aliases.txt", "r");
    if (!f) return;
    while (f.available() && numAliases < MAX_ALIASES) {
        String line = f.readStringUntil('\n');
        line.trim();
        int comma = line.indexOf(',');
        if (comma > 0) {
            String ipStr = line.substring(0, comma);
            String nameStr = line.substring(comma + 1);
            IPAddress ip;
            if (ip.fromString(ipStr) && nameStr.length()) {
                aliasList[numAliases].ip = (uint32_t)ip;
                strncpy(aliasList[numAliases].name, nameStr.c_str(), sizeof(aliasList[numAliases].name) - 1);
                aliasList[numAliases].name[sizeof(aliasList[numAliases].name) - 1] = '\0';
                numAliases++;
            }
        }
    }
    f.close();
    Serial.printf("[device_id] Loaded %d custom device aliases\n", numAliases);
}

void setDeviceAlias(const String& ipStr, const String& alias) {
    IPAddress ip;
    if (!ip.fromString(ipStr)) return;
    uint32_t uip = (uint32_t)ip;

    for (int i = 0; i < numAliases; i++) {
        if (aliasList[i].ip == uip) {
            if (alias.length() == 0) {
                // Remove alias
                for (int j = i; j < numAliases - 1; j++) {
                    aliasList[j] = aliasList[j + 1];
                }
                numAliases--;
            } else {
                strncpy(aliasList[i].name, alias.c_str(), sizeof(aliasList[i].name) - 1);
                aliasList[i].name[sizeof(aliasList[i].name) - 1] = '\0';
            }
            saveAliases();
            return;
        }
    }

    if (alias.length() > 0 && numAliases < MAX_ALIASES) {
        aliasList[numAliases].ip = uip;
        strncpy(aliasList[numAliases].name, alias.c_str(), sizeof(aliasList[numAliases].name) - 1);
        aliasList[numAliases].name[sizeof(aliasList[numAliases].name) - 1] = '\0';
        numAliases++;
        saveAliases();
    }
}

String getDeviceAlias(uint32_t ip) {
    for (int i = 0; i < numAliases; i++) {
        if (aliasList[i].ip == ip) {
            return String(aliasList[i].name);
        }
    }
    return "";
}

String getDeviceVendor(const uint8_t* mac) {
    if (!mac) return "未知設備";
    if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0) {
        return "本地區網主機";
    }

    // Check Randomized / Private MAC address (IEEE Locally Administered Address)
    // iOS 14+, Android 10+, Windows 11 default to private MAC
    if (mac[0] & 0x02) {
        return "📱 專用隨機位址 (手機/平板)";
    }

    // Match against OUI Table
    for (size_t i = 0; i < VENDOR_COUNT; i++) {
        if (mac[0] == VENDOR_TABLE[i].b0 &&
            mac[1] == VENDOR_TABLE[i].b1 &&
            mac[2] == VENDOR_TABLE[i].b2) {
            return String(VENDOR_TABLE[i].vendor);
        }
    }

    return "🌐 區域網路設備";
}

String getDeviceVendorAscii(const uint8_t* mac) {
    if (!mac) return "Unknown";
    if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0) {
        return "Localhost";
    }

    if (mac[0] & 0x02) {
        return "Private MAC";
    }

    for (size_t i = 0; i < VENDOR_COUNT; i++) {
        if (mac[0] == VENDOR_TABLE[i].b0 &&
            mac[1] == VENDOR_TABLE[i].b1 &&
            mac[2] == VENDOR_TABLE[i].b2) {
            return String(VENDOR_TABLE[i].ascii);
        }
    }

    return "LAN Device";
}

String getDeviceDisplayName(uint32_t ip, const uint8_t* mac) {
    String alias = getDeviceAlias(ip);
    if (alias.length() > 0) {
        return alias;
    }
    return getDeviceVendor(mac);
}
