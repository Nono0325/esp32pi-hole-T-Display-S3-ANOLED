#include "display.h"
#include "rm67162.h"
#include "pins_config.h"
#include "imu.h"
#include "device_id.h"
#include <TFT_eSPI.h>
#include <OneButton.h>
#include <Wire.h>
#include <qrcode.h>
#include <Preferences.h>

// Screen dimensions in landscape mode
#define SCREEN_W 536
#define SCREEN_H 240

// Modern AMOLED High-Contrast Neon Color Palette (RGB565)
#define COLOR_BG          0x0000  // Pure Black (AMOLED off)
#define COLOR_CARD        0x10A2  // Deep Graphite Slate
#define COLOR_CARD_BORDER 0x2124  // Subtle Card Border
#define COLOR_GREEN       0x07E0  // Emerald Green
#define COLOR_RED         0xF800  // Neon Coral Red
#define COLOR_CYAN        0x07FF  // Cyber Cyan
#define COLOR_AMBER       0xFD20  // Neon Amber
#define COLOR_PURPLE      0xB3DF  // Neon Lavender Purple
#define COLOR_WHITE       0xFFFF  // Pure White
#define COLOR_GRAY        0x9CD3  // Muted Gray
#define COLOR_DARKGRAY    0x4228  // Dark Charcoal

// Extern variables from main.cpp
extern uint32_t totalBlocked;
extern uint32_t totalAllowed;
extern uint32_t numHashes;
extern int numClients;
extern bool blockingOn;
extern uint32_t resumeAt;
extern String updateStatus;
extern String updateUrl;
extern bool fetchBlocklist(String url);
extern String upstreamName;
extern int numLocalDns;
extern String getTopActiveDeviceName();

// Internet connectivity sentinel status
static bool netIsOnline = true;
static int32_t netRttMs = 12;

struct WeatherData {
    float temp;
    float feelsLike;
    int humidity;
    float windSpeed;
    int weatherCode;
    char conditionStr[24];
    uint32_t lastUpdate;
    bool valid;
};

struct TopDomain {
    char domain[48];
    uint32_t count;
};

extern TopDomain topBlocked[10];
extern int numTopBlocked;
extern WeatherData currentWeather;

// TFT & Sprite objects
static TFT_eSPI tft = TFT_eSPI();
static TFT_eSprite spr = TFT_eSprite(&tft);

// Hardware Buttons
static OneButton btn1(PIN_BUTTON_1, true); // BOOT Key (GPIO 0)
static OneButton btn2(PIN_BUTTON_2, true); // User Key (GPIO 21)

// Navigation & Brightness State
uint8_t currentPage = 0;
static const uint8_t MAX_PAGES = 6;
static uint8_t brightnessLevels[] = {255, 170, 75, 15};
static uint8_t brightnessIndex = 1; // Default to 66% (170) to reduce AMOLED heat and power draw
static bool screenAsleep = false;

// Sleep Timeout & Activity State (persisted in NVS "disp")
static uint32_t sleepTimeoutSec = 300; // default 5 minutes (0 = never)
static uint32_t lastActivityMs = 0;
static Preferences dispPrefs;

// Toast Notifications
static String toastMsg = "";
static uint32_t toastExpiry = 0;

// High-Resolution Real-Time Traffic & Rate Tracking
#define WAVE_SLOTS 48
static uint16_t waveTotal[WAVE_SLOTS] = {0};
static uint16_t waveBlocked[WAVE_SLOTS] = {0};
static uint8_t waveIdx = 0;

static uint32_t secQueries = 0;
static uint32_t secBlocked = 0;
static uint32_t secBytes = 0;
static uint32_t lastSecCalc = 0;

static float currentQps = 0.0f;
static float currentBps = 0.0f;
static float peakQps = 0.0f;
static float peakBps = 0.0f;

// Touch tracking (CST816S)
static bool hasTouch = false;
static uint32_t lastTouchMs = 0;

// Function declarations
static void drawPageOverview();
static void drawPageActivity();
static void drawPageClockWeather();
static void drawPageTopBlocked();
static void drawPageSystem();
static void drawPageQR();
static void drawHeader(const char* title, uint16_t titleColor);
static void drawFooter();
static void drawToast();
static void checkTouch();

void setInternetStatus(bool online, int32_t rttMs) {
    netIsOnline = online;
    netRttMs = rttMs;
}

void resetActivityTimer() {
    lastActivityMs = millis();
}

void sleepDisplay() {
    if (screenAsleep) return;
    screenAsleep = true;
    lcd_setBrightness(0); // Cut backlight current to 0 immediately
    lcd_sleep();
    Serial.println("[display] AMOLED entered sleep mode (Display OFF & Sleep IN)");
}

void wakeupDisplay() {
    if (!screenAsleep) {
        lastActivityMs = millis();
        return;
    }
    screenAsleep = false;
    lcd_wakeup();
    lcd_setRotation(getCurrentRotation()); // Re-apply MADCTL orientation for RM67162
    lcd_setBrightness(brightnessLevels[brightnessIndex]);
    lastActivityMs = millis();
    updateDisplay(true); // force redraw immediately
    Serial.println("[display] AMOLED woke up (Display ON & Screen Restored)");
}

bool isDisplaySleeping() {
    return screenAsleep;
}

void setSleepTimeout(uint32_t seconds) {
    sleepTimeoutSec = seconds;
    lastActivityMs = millis();
    dispPrefs.begin("disp", false);
    dispPrefs.putUInt("timeout", sleepTimeoutSec);
    dispPrefs.end();
    Serial.printf("[display] Sleep timeout updated to %u sec\n", sleepTimeoutSec);
}

uint32_t getSleepTimeout() {
    return sleepTimeoutSec;
}

void recordQueryActivity(bool blocked, uint16_t bytes) {
    if (bytes == 0) bytes = 64;
    secQueries++;
    secBytes += bytes;
    if (blocked) {
        secBlocked++;
    }
}

float getCurrentQPS() { return currentQps; }
float getCurrentBPS() { return currentBps; }
float getPeakQPS() { return peakQps; }
float getPeakBPS() { return peakBps; }

void displayToast(const char* message, uint32_t durationMs) {
    toastMsg = message;
    toastExpiry = millis() + durationMs;
    resetActivityTimer();
    if (screenAsleep) {
        wakeupDisplay();
    }
}

void setDisplayBrightness(uint8_t brightness) {
    lcd_setBrightness(brightness);
    resetActivityTimer();
}

void nextPage() {
    currentPage = (currentPage + 1) % MAX_PAGES;
    resetActivityTimer();
}

void setDisplayPage(uint8_t page) {
    if (page < MAX_PAGES) {
        currentPage = page;
        if (screenAsleep) {
            wakeupDisplay();
        } else {
            resetActivityTimer();
        }
    }
}

static void onBtn1Click() {
    resetActivityTimer();
    if (screenAsleep) {
        wakeupDisplay();
        return;
    }
    nextPage();
}

static void onBtn1LongPress() {
    resetActivityTimer();
    if (screenAsleep) {
        wakeupDisplay();
        return;
    }
    displayToast("UPDATING RULES...", 3000);
    if (updateUrl.length()) {
        fetchBlocklist(updateUrl);
        displayToast(updateStatus.c_str(), 3500);
    } else {
        displayToast("NO UPDATE URL SET", 2500);
    }
}

static void onBtn2Click() {
    resetActivityTimer();
    if (screenAsleep) {
        wakeupDisplay();
        return;
    }
    // Toggle blocking state: Active <-> Paused 5 mins (300s)
    if (blockingOn) {
        blockingOn = false;
        resumeAt = millis() + 300000UL;
        displayToast("PAUSED (5 MINS)", 2500);
    } else {
        blockingOn = true;
        resumeAt = 0;
        displayToast("SHIELD RESUMED", 2500);
    }
}

static void onBtn2LongPress() {
    resetActivityTimer();
    if (screenAsleep) {
        wakeupDisplay();
        return;
    }
    brightnessIndex = (brightnessIndex + 1) % 4;
    lcd_setBrightness(brightnessLevels[brightnessIndex]);
    char bStr[32];
    snprintf(bStr, sizeof(bStr), "BRIGHTNESS: %u%%", (brightnessLevels[brightnessIndex] * 100) / 255);
    displayToast(bStr, 2000);
}

// ==================== NonoWuW Custom Cyber Logo & Boot Animation ====================
static void drawNonoWuWLogo(int cx, int cy, float scale, uint16_t glowColor, uint8_t expression = 0, bool drawFace = true) {
    if (scale <= 0.05f) return;
    int w = (int)(270 * scale);
    int h = (int)(106 * scale);
    int x = cx - w / 2;
    int y = cy - h / 2;

    // 1. Outer Deep Tech Shield with Neon Glow Rim
    spr.fillRoundRect(x, y, w, h, (int)(18 * scale), 0x0842); // Deep techno carbon
    spr.drawRoundRect(x, y, w, h, (int)(18 * scale), glowColor);
    if (scale > 0.45f) {
        spr.drawRoundRect(x + 2, y + 2, w - 4, h - 4, (int)(16 * scale), 0x1925);
    }
    if (scale > 0.75f) {
        spr.drawRoundRect(x + 4, y + 4, w - 8, h - 8, (int)(14 * scale), 0x0A44);
    }

    // 2. Stylized Cyber Cat-Ears / Dual Quantum Antenna Fins
    int earW = (int)(26 * scale);
    int earH = (int)(20 * scale);
    int ear1X = x + (int)(36 * scale);
    int ear2X = x + w - (int)(36 * scale) - earW;

    // Left Antenna Ear
    spr.fillTriangle(ear1X, y, ear1X + earW / 2, y - earH, ear1X + earW, y, glowColor);
    spr.fillTriangle(ear1X + 3, y, ear1X + earW / 2, y - earH + 4, ear1X + earW - 3, y, 0x0842);
    // Left glowing tip node
    spr.fillCircle(ear1X + earW / 2, y - earH, (int)(2 * scale) + 1, COLOR_WHITE);

    // Right Antenna Ear
    spr.fillTriangle(ear2X, y, ear2X + earW / 2, y - earH, ear2X + earW, y, glowColor);
    spr.fillTriangle(ear2X + 3, y, ear2X + earW / 2, y - earH + 4, ear2X + earW - 3, y, 0x0842);
    // Right glowing tip node
    spr.fillCircle(ear2X + earW / 2, y - earH, (int)(2 * scale) + 1, COLOR_WHITE);

    // Antenna signal pulse notches
    if (scale >= 0.7f) {
        spr.drawFastHLine(ear1X - 6, y - earH / 2, 4, glowColor);
        spr.drawFastHLine(ear1X - 9, y - earH / 2 - 4, 5, glowColor);
        spr.drawFastHLine(ear2X + earW + 2, y - earH / 2, 4, glowColor);
        spr.drawFastHLine(ear2X + earW + 4, y - earH / 2 - 4, 5, glowColor);
    }

    // 3. High-Tech Corner Brackets & Crosshairs
    if (scale >= 0.65f) {
        int blen = (int)(8 * scale);
        // Top-Left bracket
        spr.drawFastHLine(x - 8, y + 6, blen, glowColor);
        spr.drawFastVLine(x - 8, y + 6, blen, glowColor);
        // Top-Right bracket
        spr.drawFastHLine(x + w + 8 - blen, y + 6, blen, glowColor);
        spr.drawFastVLine(x + w + 8, y + 6, blen, glowColor);
        // Bottom-Left bracket
        spr.drawFastHLine(x - 8, y + h - 6, blen, glowColor);
        spr.drawFastVLine(x - 8, y + h - 6 - blen, blen, glowColor);
        // Bottom-Right bracket
        spr.drawFastHLine(x + w + 8 - blen, y + h - 6, blen, glowColor);
        spr.drawFastVLine(x + w + 8, y + h - 6 - blen, blen, glowColor);

        // Tech crosshairs
        spr.drawFastHLine(x + 12, cy, 8, 0x2965);
        spr.drawFastHLine(x + w - 20, cy, 8, 0x2965);
    }

    // 4. Logo Typography, Visor & Mascot Expression
    if (drawFace && scale >= 0.65f) {
        spr.setTextDatum(MC_DATUM);

        // Top mini status line
        spr.setTextColor(0x5AEB, 0x0842); // Muted cyber cyan
        spr.drawString("• NONOWUW QUANTUM SHIELD •", cx, cy - (int)(36 * scale), 1);

        // Bold Title: NonoWuW with 3D drop-shadow
        spr.setTextColor(0x0126, 0x0842); // Deep drop shadow
        spr.drawString("NonoWuW", cx + 2, cy - (int)(14 * scale), 4);
        spr.setTextColor(COLOR_CYAN, 0x0842); // Primary electric cyan
        spr.drawString("NonoWuW", cx, cy - (int)(16 * scale), 4);

        // Subtitle badge: Glowing Visor Pill
        int badgeW = (int)(200 * scale);
        int badgeH = (int)(24 * scale);
        int badgeX = cx - badgeW / 2;
        int badgeY = cy + (int)(11 * scale);

        spr.fillRoundRect(badgeX, badgeY, badgeW, badgeH, (int)(6 * scale) + 1, 0x10E4);
        spr.drawRoundRect(badgeX, badgeY, badgeW, badgeH, (int)(6 * scale) + 1, glowColor);

        // Mascot expressions
        const char* faceStr = "( ✧ WuW ✧ )";
        uint16_t faceColor = 0xD81F; // Neon magenta
        if (expression == 1) {
            faceStr = "( ★ ω ★ )";
            faceColor = COLOR_AMBER;
        } else if (expression == 2) {
            faceStr = "( ✧ ω < )";
            faceColor = COLOR_CYAN;
        } else if (expression == 3) {
            faceStr = "( ˘ ω ˘ )";
            faceColor = COLOR_GREEN;
        }

        spr.setTextColor(faceColor, 0x10E4);
        spr.drawString(faceStr, cx - (int)(46 * scale), badgeY + badgeH / 2, 2);

        // Divider in badge
        spr.drawFastVLine(cx + (int)(10 * scale), badgeY + 3, badgeH - 6, 0x31E7);

        // Subtitle tag
        spr.setTextColor(COLOR_GREEN, 0x10E4);
        spr.drawString("AD-SHIELD", cx + (int)(54 * scale), badgeY + badgeH / 2, 2);
    }
}

void playBootAnimation() {
    int cx = SCREEN_W / 2;
    int cy = 78;

    // ===== Phase 1: Laser Scanline awakening (12 frames) =====
    for (int i = 1; i <= 12; i++) {
        spr.fillSprite(COLOR_BG);
        int lineLen = (i * SCREEN_W) / 12;
        int x0 = cx - lineLen / 2;

        spr.drawFastHLine(x0, 120, lineLen, COLOR_CYAN);
        spr.drawFastHLine(x0, 119, lineLen, 0xF81F);
        if (lineLen > 40) {
            spr.drawFastHLine(x0 + 20, 121, lineLen - 40, 0x07E0);
        }

        int vSpread = i * 8;
        spr.drawFastHLine(cx - lineLen / 3, 120 - vSpread, (lineLen * 2) / 3, 0x1185);
        spr.drawFastHLine(cx - lineLen / 3, 120 + vSpread, (lineLen * 2) / 3, 0x1185);

        if (13 - i > 0) {
            spr.fillCircle(cx, 120, 13 - i, COLOR_WHITE);
        }

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(0x5AEB, COLOR_BG);
        spr.drawString("INITIALIZING QUANTUM CORE...", cx, 205, 2);

        lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
        delay(18);
    }

    // ===== Phase 2: Matrix Digital Stream & Concentric Rings (14 frames) =====
    const char* matrixTokens[] = {"0x7F", "DNS", "S3", "AMOLED", "NONO", "WUW", "120K", "ESP32", "SHIELD", "IPv6"};
    for (int i = 1; i <= 14; i++) {
        spr.fillSprite(COLOR_BG);

        int r = (int)(20 + i * 9);
        spr.drawCircle(cx, cy, r, (i % 2 == 0) ? 0x1A27 : 0x08A2);
        if (r > 30) spr.drawCircle(cx, cy, r - 15, 0x1144);

        spr.setTextDatum(MC_DATUM);
        for (int col = 0; col < 8; col++) {
            int colX = 40 + col * 65;
            int tokenY = (col * 37 + i * 22) % 240;
            spr.setTextColor((i % 2 == 0) ? COLOR_GREEN : 0x05E0, COLOR_BG);
            spr.drawString(matrixTokens[(col + i) % 10], colX, tokenY, 1);
        }

        float scale = 0.25f + 0.45f * ((float)i / 14.0f);
        uint16_t glow = (i % 2 == 0) ? COLOR_CYAN : 0xF81F;
        drawNonoWuWLogo(cx, cy, scale, glow, 0, i >= 10);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_GRAY, COLOR_BG);
        spr.drawString("CALIBRATING AMOLED 60FPS DISPLAY ENGINE...", cx, 198, 2);

        lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
        delay(22);
    }

    // ===== Phase 3: Holographic Logo Materialization & Visor Awakening (18 frames) =====
    for (int i = 1; i <= 18; i++) {
        spr.fillSprite(COLOR_BG);

        float scale = 0.7f + 0.3f * ((float)i / 18.0f);
        uint16_t glow = (i >= 15) ? COLOR_GREEN : ((i % 4 < 2) ? COLOR_CYAN : 0xF81F);
        uint8_t expr = (i < 6) ? 3 : ((i < 12) ? 0 : 1);

        int r = (int)(i * 12);
        if (r < 220) spr.drawCircle(cx, cy, r, 0x1A47);

        drawNonoWuWLogo(cx, cy, scale, glow, expr, true);

        for (int p = 0; p < 6; p++) {
            float ang = (float)p * 1.047f + (float)i * 0.15f;
            int px = cx + (int)(cos(ang) * (140 + i * 4));
            int py = cy + (int)(sin(ang) * (60 + i * 2));
            if (px >= 2 && px < SCREEN_W - 2 && py >= 2 && py < SCREEN_H - 2) {
                spr.fillCircle(px, py, 2, glow);
            }
        }

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_CYAN, COLOR_BG);
        spr.drawString("ARMING NONOWUW CYBER SENTINEL...", cx, 198, 2);

        lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
        delay(22);
    }

    // ===== Phase 4: Cyber Diagnostics & Smooth Loading Bar (36 frames) =====
    int barW = 360;
    int barH = 10;
    int barX = cx - barW / 2;
    int barY = 160;

    const char* diagMsgs[] = {
        ">> [01/05] XTENSA DUAL-CORE LX7 240MHz ... READY",
        ">> [02/05] RM67162 AMOLED 536x240 QSPI ... 60FPS LOCKED",
        ">> [03/05] 16MB FLASH + 8MB OPI PSRAM ... INITIALIZED",
        ">> [04/05] QUANTUM DNS FILTER ... 120,000+ RULES LOADED",
        ">> [05/05] PING SENTINEL WAN MONITOR ... SYSTEM ARMED",
        ">> ★ WELCOME BACK, NONOWUW! CYBER SHIELD ONLINE (WuW) ★"
    };

    for (int f = 1; f <= 36; f++) {
        spr.fillSprite(COLOR_BG);

        uint16_t glow = (f >= 30) ? COLOR_GREEN : ((f % 6 < 3) ? COLOR_CYAN : 0xF81F);
        uint8_t expr = (f >= 30) ? 1 : ((f % 10 < 5) ? 0 : 2);
        drawNonoWuWLogo(cx, cy, 1.0f, glow, expr, true);

        // Progress Bar Frame with tech accents
        spr.drawRoundRect(barX - 2, barY - 2, barW + 4, barH + 4, 5, COLOR_CARD_BORDER);
        spr.drawFastHLine(barX - 6, barY + barH / 2, 4, glow);
        spr.drawFastHLine(barX + barW + 2, barY + barH / 2, 4, glow);

        int fillW = (f * barW) / 36;
        if (fillW > 0) {
            uint16_t barFillColor = (f >= 30) ? COLOR_GREEN : COLOR_CYAN;
            spr.fillRoundRect(barX, barY, fillW, barH, 3, barFillColor);
            spr.drawFastHLine(barX, barY, fillW, COLOR_WHITE);

            int tipX = barX + fillW - 1;
            spr.fillCircle(tipX, barY + barH / 2, 4, COLOR_WHITE);
            spr.drawCircle(tipX, barY + barH / 2, 6, barFillColor);
        }

        // Percentage
        int pct = (f * 100) / 36;
        char pctStr[16];
        snprintf(pctStr, sizeof(pctStr), "%3d%%", pct);
        spr.setTextDatum(ML_DATUM);
        spr.setTextColor((f >= 30) ? COLOR_GREEN : COLOR_CYAN, COLOR_BG);
        spr.drawString(pctStr, barX + barW + 12, barY + barH / 2, 2);

        // Diagnostic Step Message
        int stepIdx = (f * 5) / 36;
        if (f >= 32) stepIdx = 5;
        if (stepIdx > 5) stepIdx = 5;
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor((stepIdx == 5) ? COLOR_GREEN : COLOR_GRAY, COLOR_BG);
        spr.drawString(diagMsgs[stepIdx], cx, 195, 2);

        spr.setTextColor(COLOR_DARKGRAY, COLOR_BG);
        spr.drawString("LilyGO T-Display-S3 AMOLED  |  NonoWuW Exclusive Edition", cx, 224, 1);

        lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
        delay(24);
    }

    // ===== Phase 5: Quick Dissolve Flare (4 frames) =====
    for (int i = 0; i < 4; i++) {
        spr.fillSprite(COLOR_BG);
        drawNonoWuWLogo(cx, cy, 1.0f, (i % 2 == 0) ? COLOR_WHITE : COLOR_GREEN, 1, true);
        lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
        delay(30);
    }

    delay(200);
}

void replayBootAnimation() {
    Serial.println("[display] Replaying NonoWuW Cyber Boot Animation!");
    playBootAnimation();
    updateDisplay(true); // Restore dashboard immediately
}

void initDisplay() {
    // 1. Enable AMOLED Screen Power (GPIO 38 HIGH)
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    delay(50);

    // 2. Initialize RM67162 AMOLED Driver
    rm67162_init();
    lcd_setRotation(1); // 536x240 landscape
    lcd_setBrightness(brightnessLevels[brightnessIndex]);

    // 3. Create Full Frame Sprite Buffer in PSRAM / Internal RAM
    spr.createSprite(SCREEN_W, SCREEN_H);
    spr.setSwapBytes(true);

    // 4. Configure Buttons
    btn1.attachClick(onBtn1Click);
    btn1.attachLongPressStart(onBtn1LongPress);
    btn2.attachClick(onBtn2Click);
    btn2.attachDoubleClick(flipScreen180); // Double-click Button 2 to flip 180 degrees
    btn2.attachLongPressStart(onBtn2LongPress);

    // 5. Initialize IMU Gravity Sensor (Auto-detects MPU6050, QMA6100, LIS3DH, BMA423, etc.)
    initIMU();

    // 6. Detect CST816S Touch Controller
    Wire.beginTransmission(0x15);
    if (Wire.endTransmission() == 0) {
        hasTouch = true;
        Serial.println("[amoled] CST816 Touch Controller detected!");
    } else {
        hasTouch = false;
        Serial.println("[amoled] Non-Touch Edition (Hardware Buttons Active)");
    }

    // 7. Load Sleep Timeout configuration from NVS
    dispPrefs.begin("disp", true);
    sleepTimeoutSec = dispPrefs.getUInt("timeout", 300);
    dispPrefs.end();
    lastActivityMs = millis();
    Serial.printf("[display] Sleep timeout configured: %u sec\n", sleepTimeoutSec);

    // 8. Play NonoWuW Exclusive Boot Animation & Custom Logo
    playBootAnimation();
}

static void checkTouch() {
    if (!hasTouch) return;
    if (millis() - lastTouchMs < 300) return;

    Wire.beginTransmission(0x15);
    Wire.write(0x01); // Register 0x01: Gesture ID
    if (Wire.endTransmission() != 0) return;

    if (Wire.requestFrom((uint8_t)0x15, (uint8_t)6) == 6) {
        uint8_t gesture = Wire.read();
        uint8_t points  = Wire.read();
        uint8_t xHigh   = Wire.read() & 0x0F;
        uint8_t xLow    = Wire.read();
        uint8_t yHigh   = Wire.read() & 0x0F;
        uint8_t yLow    = Wire.read();

        if (points > 0) {
            lastTouchMs = millis();
            resetActivityTimer();
            if (screenAsleep) {
                wakeupDisplay();
                return;
            }
            uint16_t tx = (xHigh << 8) | xLow;
            uint16_t ty = (yHigh << 8) | yLow;

            // Gesture: 0x01 Slide Up, 0x02 Slide Down, 0x03 Slide Left, 0x04 Slide Right, 0x05 Single Click
            if (gesture == 0x03 || (gesture == 0x05 && tx > SCREEN_W * 2 / 3)) {
                nextPage();
            } else if (gesture == 0x04 || (gesture == 0x05 && tx < SCREEN_W / 3)) {
                currentPage = (currentPage + MAX_PAGES - 1) % MAX_PAGES;
            } else if (gesture == 0x05) {
                // Center tap: toggle blocking
                onBtn2Click();
            }
        }
    }
}

void updateDisplay(bool force) {
    if (screenAsleep) {
        // Direct zero-latency hardware button check for instant wakeup on BOOT or User key
        if (digitalRead(PIN_BUTTON_1) == LOW || digitalRead(PIN_BUTTON_2) == LOW) {
            delay(15); // Debounce
            if (digitalRead(PIN_BUTTON_1) == LOW || digitalRead(PIN_BUTTON_2) == LOW) {
                Serial.println("[display] Physical button pressed -> Instant Hardware Wakeup!");
                wakeupDisplay();
                while (digitalRead(PIN_BUTTON_1) == LOW || digitalRead(PIN_BUTTON_2) == LOW) {
                    delay(10);
                }
                delay(20);
                return;
            }
        }
    }

    // Tick buttons and touch controller (hardware wakeup triggers)
    btn1.tick();
    btn2.tick();
    checkTouch();
    if (!screenAsleep) {
        updateIMU(); // Skip I2C accelerometer polling when screen is sleeping to save power
    }

    // Inactivity sleep check
    if (sleepTimeoutSec > 0 && !screenAsleep && (millis() - lastActivityMs >= sleepTimeoutSec * 1000UL)) {
        sleepDisplay();
    }

    if (screenAsleep) return;

    // 1-second high-resolution traffic rate calculation & rolling waveform
    uint32_t nowMs = millis();
    if (nowMs - lastSecCalc >= 1000UL) {
        float dt = (float)(nowMs - lastSecCalc) / 1000.0f;
        lastSecCalc = nowMs;

        currentQps = (float)secQueries / dt;
        currentBps = (float)secBytes / dt;
        if (currentQps > peakQps) peakQps = currentQps;
        if (currentBps > peakBps) peakBps = currentBps;

        waveIdx = (waveIdx + 1) % WAVE_SLOTS;
        waveTotal[waveIdx] = (uint16_t)(currentQps + 0.5f);
        waveBlocked[waveIdx] = (uint16_t)((float)secBlocked / dt + 0.5f);

        secQueries = 0;
        secBlocked = 0;
        secBytes = 0;
    }

    static uint32_t lastRender = 0;
    if (!force && millis() - lastRender < 350) return; // ~3 FPS smooth dashboard refresh
    lastRender = millis();

    spr.fillSprite(COLOR_BG);

    switch (currentPage) {
        case 0: drawPageOverview();      break;
        case 1: drawPageActivity();      break;
        case 2: drawPageClockWeather();  break;
        case 3: drawPageTopBlocked();    break;
        case 4: drawPageSystem();        break;
        case 5: drawPageQR();            break;
    }

    drawToast();
    lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
}

static void drawHeader(const char* title, uint16_t titleColor) {
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(titleColor, COLOR_BG);
    spr.drawString(title, 12, 6, 2);

    // WiFi Info & IP & Ping Latency / WAN Alert
    spr.setTextDatum(TR_DATUM);
    if (!netIsOnline) {
        bool flash = ((millis() / 500) % 2) == 0;
        spr.setTextColor(flash ? COLOR_RED : COLOR_AMBER, COLOR_BG);
        spr.drawString("! WAN OFFLINE !", SCREEN_W - 12, 6, 2);
    } else {
        String netInfo = "";
        if (WiFi.status() == WL_CONNECTED) {
            char pingStr[16];
            if (netRttMs >= 0) snprintf(pingStr, sizeof(pingStr), "[%ldms] ", (long)netRttMs);
            else strcpy(pingStr, "");
            netInfo = String(pingStr) + WiFi.SSID() + " (" + String(WiFi.RSSI()) + "dBm)  " + WiFi.localIP().toString();
        } else {
            netInfo = "WiFi: Disconnected";
        }
        spr.setTextColor(COLOR_GRAY, COLOR_BG);
        spr.drawString(netInfo, SCREEN_W - 12, 6, 2);
    }

    // Sleek header divider line
    spr.drawFastHLine(12, 26, SCREEN_W - 24, COLOR_CARD_BORDER);
}

static void drawFooter() {
    // Divider
    spr.drawFastHLine(12, 206, SCREEN_W - 24, COLOR_CARD_BORDER);

    // Help prompts
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_BG);
    spr.drawString("[KEY 1] Page (1-6)  [KEY 2] Pause | 2x: Flip 180", 14, 215, 2);

    // Page indicator dots (6 dots)
    spr.setTextDatum(TR_DATUM);
    for (int i = 0; i < MAX_PAGES; i++) {
        int dotX = SCREEN_W - 96 + (i * 14);
        if (i == currentPage) {
            spr.fillCircle(dotX, 222, 4, COLOR_CYAN);
        } else {
            spr.drawCircle(dotX, 222, 3, COLOR_DARKGRAY);
        }
    }
}

static void drawPageOverview() {
    drawHeader("NonoWuW SHIELD", COLOR_CYAN);

    // ---- Status Banner (y: 34 to 68) ----
    uint16_t statusBg = blockingOn ? 0x0284 : 0x31E0;
    uint16_t statusText = blockingOn ? COLOR_GREEN : COLOR_AMBER;
    spr.fillRoundRect(12, 34, 230, 32, 8, statusBg);
    spr.drawRoundRect(12, 34, 230, 32, 8, statusText);

    spr.fillCircle(28, 50, 5, statusText);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(statusText, statusBg);
    if (blockingOn) {
        spr.drawString("NonoWuW ACTIVE", 42, 50, 2);
    } else {
        if (resumeAt) {
            uint32_t rem = (resumeAt > millis()) ? (resumeAt - millis()) / 1000 : 0;
            char pStr[32];
            snprintf(pStr, sizeof(pStr), "PAUSED %02lu:%02lu", rem / 60, rem % 60);
            spr.drawString(pStr, 42, 50, 2);
        } else {
            spr.drawString("PAUSED INDEFINITE", 42, 50, 2);
        }
    }

    // Right banner: Rules Count & Upstream
    spr.fillRoundRect(252, 34, SCREEN_W - 264, 32, 8, COLOR_CARD);
    spr.drawRoundRect(252, 34, SCREEN_W - 264, 32, 8, COLOR_CARD_BORDER);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char rulesStr[48];
    snprintf(rulesStr, sizeof(rulesStr), "Rules: %u domains", numHashes);
    spr.drawString(rulesStr, 264, 50, 2);

    spr.setTextDatum(MR_DATUM);
    char upInfo[64];
    if (netIsOnline && netRttMs >= 0) {
        snprintf(upInfo, sizeof(upInfo), "Upstream: %s (%ldms)", upstreamName.c_str(), (long)netRttMs);
        spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    } else if (!netIsOnline) {
        snprintf(upInfo, sizeof(upInfo), "Upstream: WAN OFFLINE");
        spr.setTextColor(COLOR_RED, COLOR_CARD);
    } else {
        snprintf(upInfo, sizeof(upInfo), "Upstream: %s", upstreamName.c_str());
        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    }
    spr.drawString(upInfo, SCREEN_W - 24, 50, 2);

    // ---- 3 Stat Cards (y: 74 to 198) ----
    int cardY = 74;
    int cardH = 124;
    int cardW = 164;
    int gap = 10;
    int cardX1 = 12;
    int cardX2 = cardX1 + cardW + gap;
    int cardX3 = cardX2 + cardW + gap;

    uint32_t totalQueries = totalBlocked + totalAllowed;
    float blockRate = (totalQueries > 0) ? ((float)totalBlocked * 100.0f / totalQueries) : 0.0f;

    // --- Card 1: Blocked Queries ---
    spr.fillRoundRect(cardX1, cardY, cardW, cardH, 10, COLOR_CARD);
    spr.drawRoundRect(cardX1, cardY, cardW, cardH, 10, COLOR_CARD_BORDER);
    spr.drawFastHLine(cardX1 + 10, cardY + 2, cardW - 20, COLOR_RED);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("BLOCKED ADS", cardX1 + 12, cardY + 12, 1);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_RED, COLOR_CARD);
    char bVal[24];
    snprintf(bVal, sizeof(bVal), "%lu", totalBlocked);
    spr.drawString(bVal, cardX1 + 12, cardY + 28, 4);

    // Block rate pill badge
    spr.fillRoundRect(cardX1 + 12, cardY + 76, 96, 22, 6, 0x3000);
    spr.setTextColor(COLOR_RED, 0x3000);
    spr.setTextDatum(MC_DATUM);
    char pctStr[20];
    snprintf(pctStr, sizeof(pctStr), "%.1f%% RATE", blockRate);
    spr.drawString(pctStr, cardX1 + 60, cardY + 87, 2);

    // --- Card 2: Total Traffic ---
    spr.fillRoundRect(cardX2, cardY, cardW, cardH, 10, COLOR_CARD);
    spr.drawRoundRect(cardX2, cardY, cardW, cardH, 10, COLOR_CARD_BORDER);
    spr.drawFastHLine(cardX2 + 10, cardY + 2, cardW - 20, COLOR_CYAN);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("TOTAL QUERIES", cardX2 + 12, cardY + 12, 1);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    char qVal[24];
    snprintf(qVal, sizeof(qVal), "%lu", totalQueries);
    spr.drawString(qVal, cardX2 + 12, cardY + 28, 4);

    // Progress bar for allowed vs blocked
    int barW = cardW - 24;
    spr.drawRoundRect(cardX2 + 12, cardY + 74, barW, 8, 3, COLOR_DARKGRAY);
    int fillW = (int)((float)barW * (blockRate / 100.0f));
    if (fillW > 0) spr.fillRoundRect(cardX2 + 12, cardY + 74, fillW, 8, 3, COLOR_RED);
    if (barW - fillW > 0) spr.fillRoundRect(cardX2 + 12 + fillW, cardY + 74, barW - fillW, 8, 3, COLOR_GREEN);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    char aStr[32];
    snprintf(aStr, sizeof(aStr), "Allowed: %lu", totalAllowed);
    spr.drawString(aStr, cardX2 + 12, cardY + 92, 1);

    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    char rStr[32];
    snprintf(rStr, sizeof(rStr), "Rate: %.1f q/s", currentQps);
    spr.drawString(rStr, cardX2 + cardW - 12, cardY + 92, 1);

    // --- Card 3: Network Clients ---
    spr.fillRoundRect(cardX3, cardY, cardW, cardH, 10, COLOR_CARD);
    spr.drawRoundRect(cardX3, cardY, cardW, cardH, 10, COLOR_CARD_BORDER);
    spr.drawFastHLine(cardX3 + 10, cardY + 2, cardW - 20, COLOR_PURPLE);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("CONNECTED CLIENTS", cardX3 + 12, cardY + 12, 1);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_PURPLE, COLOR_CARD);
    char cVal[24];
    snprintf(cVal, sizeof(cVal), "%d", numClients);
    spr.drawString(cVal, cardX3 + 12, cardY + 28, 4);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    String topDev = getTopActiveDeviceName();
    if (topDev.length() > 0) {
        if (topDev.length() > 18) topDev = topDev.substring(0, 18);
        spr.setTextColor(COLOR_CYAN, COLOR_CARD);
        spr.drawString(topDev, cardX3 + 12, cardY + 76, 2);
    } else {
        spr.drawString("Active LAN Devices", cardX3 + 12, cardY + 76, 2);
    }
    char upStr[32];
    snprintf(upStr, sizeof(upStr), "Uptime: %luh", (millis() / 3600000UL));
    spr.drawString(upStr, cardX3 + 12, cardY + 96, 1);

    drawFooter();
}

static void drawPageActivity() {
    drawHeader("REAL-TIME TRAFFIC & SPEED", COLOR_CYAN);

    // ==================== TOP: 3 Live Speed Cards (y: 34 to 88, h: 54) ====================
    int cardY = 34;
    int cardH = 54;
    int cardW = 164;
    int gap = 10;
    int c1X = 12;
    int c2X = c1X + cardW + gap; // 186
    int c3X = c2X + cardW + gap; // 360

    // --- Card 1: Current Query & Transfer Speed ---
    spr.fillRoundRect(c1X, cardY, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(c1X, cardY, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.drawFastHLine(c1X + 8, cardY + 2, cardW - 16, COLOR_CYAN);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("CURRENT SPEED", c1X + 10, cardY + 8, 1);

    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    char qpsStr[24];
    snprintf(qpsStr, sizeof(qpsStr), "%.1f", currentQps);
    spr.drawString(qpsStr, c1X + 10, cardY + 22, 4);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    spr.drawString("q/s", c1X + 82, cardY + 28, 2);

    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char bwStr[32];
    snprintf(bwStr, sizeof(bwStr), "%.1f KB/s", currentBps / 1024.0f);
    spr.drawString(bwStr, c1X + cardW - 10, cardY + 36, 1);

    // --- Card 2: Peak Records ---
    spr.fillRoundRect(c2X, cardY, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(c2X, cardY, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.drawFastHLine(c2X + 8, cardY + 2, cardW - 16, COLOR_AMBER);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("PEAK SPEED", c2X + 10, cardY + 8, 1);

    spr.setTextColor(COLOR_AMBER, COLOR_CARD);
    char peakQpsStr[24];
    snprintf(peakQpsStr, sizeof(peakQpsStr), "%.1f", peakQps);
    spr.drawString(peakQpsStr, c2X + 10, cardY + 22, 4);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_AMBER, COLOR_CARD);
    spr.drawString("q/s", c2X + 82, cardY + 28, 2);

    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char peakBwStr[32];
    snprintf(peakBwStr, sizeof(peakBwStr), "Peak: %.1f KB/s", peakBps / 1024.0f);
    spr.drawString(peakBwStr, c2X + cardW - 10, cardY + 36, 1);

    // --- Card 3: Block Speed & Ratio ---
    spr.fillRoundRect(c3X, cardY, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(c3X, cardY, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.drawFastHLine(c3X + 8, cardY + 2, cardW - 16, COLOR_RED);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("BLOCK SHIELD", c3X + 10, cardY + 8, 1);

    uint32_t totalQueries = totalBlocked + totalAllowed;
    float blockRate = (totalQueries > 0) ? ((float)totalBlocked * 100.0f / totalQueries) : 0.0f;

    spr.setTextColor(COLOR_RED, COLOR_CARD);
    char blkRateStr[24];
    snprintf(blkRateStr, sizeof(blkRateStr), "%.1f", blockRate);
    spr.drawString(blkRateStr, c3X + 10, cardY + 22, 4);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_RED, COLOR_CARD);
    spr.drawString("%", c3X + 82, cardY + 28, 2);

    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char blkTotalStr[32];
    snprintf(blkTotalStr, sizeof(blkTotalStr), "Blocked: %lu", totalBlocked);
    spr.drawString(blkTotalStr, c3X + cardW - 10, cardY + 36, 1);

    // ==================== BOTTOM: 48-Second Oscillogram (y: 94 to 200, h: 106) ====================
    int graphX = 12;
    int graphY = 94;
    int graphW = SCREEN_W - 24; // 512
    int graphH = 106;

    spr.fillRoundRect(graphX, graphY, graphW, graphH, 8, COLOR_CARD);
    spr.drawRoundRect(graphX, graphY, graphW, graphH, 8, COLOR_CARD_BORDER);

    // Header inside Oscillogram panel
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(0x5AEB, COLOR_CARD); // Tech Cyan
    spr.drawString("• 48-SEC REAL-TIME TRAFFIC OSCILLOGRAM (1-SEC/DIV)", graphX + 10, graphY + 8, 1);

    // Legend
    spr.setTextDatum(TR_DATUM);
    spr.fillCircle(graphX + graphW - 180, graphY + 12, 3, COLOR_GREEN);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("Pass", graphX + graphW - 152, graphY + 8, 1);

    spr.fillCircle(graphX + graphW - 140, graphY + 12, 3, COLOR_RED);
    spr.drawString("Block", graphX + graphW - 110, graphY + 8, 1);

    spr.drawFastHLine(graphX + graphW - 95, graphY + 12, 10, COLOR_CYAN);
    spr.drawString("QPS", graphX + graphW - 65, graphY + 8, 1);

    // Live blinking indicator
    bool livePulse = ((millis() / 500) % 2) == 0;
    spr.fillCircle(graphX + graphW - 48, graphY + 12, 3, livePulse ? COLOR_GREEN : 0x03E0);
    spr.setTextColor(livePulse ? COLOR_GREEN : COLOR_DARKGRAY, COLOR_CARD);
    spr.drawString("LIVE", graphX + graphW - 10, graphY + 8, 1);

    // Determine dynamic maximum QPS in 48-second window
    uint16_t maxQ = 6; // minimum scale
    for (int i = 0; i < WAVE_SLOTS; i++) {
        if (waveTotal[i] > maxQ) maxQ = waveTotal[i];
    }

    // Grid baseline & horizontal dividers
    int baseLineY = graphY + 82; // Y: 176
    int maxWaveH = 50;           // maximum bar height up to Y: 126
    spr.drawFastHLine(graphX + 10, baseLineY, graphW - 20, COLOR_DARKGRAY);
    spr.drawFastHLine(graphX + 10, baseLineY - maxWaveH / 2, graphW - 20, 0x1925);
    spr.drawFastHLine(graphX + 10, baseLineY - maxWaveH, graphW - 20, 0x1925);

    // Max scale label
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(0x5AEB, COLOR_CARD);
    char scaleStr[16];
    snprintf(scaleStr, sizeof(scaleStr), "%uq", maxQ);
    spr.drawString(scaleStr, graphX + 12, baseLineY - maxWaveH - 4, 1);

    // Draw 48 Bars & Trend Line
    int startX = graphX + 16; // 28
    int barW = 7;
    int prevX = -1;
    int prevY = -1;

    for (int i = 0; i < WAVE_SLOTS; i++) {
        int slot = (waveIdx + 1 + i) % WAVE_SLOTS;
        uint16_t tot = waveTotal[slot];
        uint16_t blk = waveBlocked[slot];
        uint16_t alw = (tot >= blk) ? (tot - blk) : 0;

        int totalH = (int)((float)tot / maxQ * maxWaveH);
        int blockedH = (int)((float)blk / maxQ * maxWaveH);
        int allowedH = totalH - blockedH;

        int bx = startX + i * 10;
        int py = baseLineY - totalH;
        int px = bx + 3;

        if (totalH > 0) {
            // Allowed portion (Green)
            if (allowedH > 0) {
                spr.fillRect(bx, baseLineY - allowedH, barW, allowedH, COLOR_GREEN);
            }
            // Blocked portion (Red)
            if (blockedH > 0) {
                spr.fillRect(bx, baseLineY - totalH, barW, blockedH, COLOR_RED);
            }
        } else {
            // Idle dot at baseline
            spr.drawFastHLine(bx, baseLineY, barW, 0x2124);
        }

        // Cyan trend line connecting top of wave
        if (prevX >= 0) {
            spr.drawLine(prevX, prevY, px, py, COLOR_CYAN);
        }
        prevX = px;
        prevY = py;

        // Current / Latest Slot glowing marker
        if (i == WAVE_SLOTS - 1) {
            spr.fillCircle(px, py, 3, COLOR_WHITE);
            spr.drawCircle(px, py, 5, COLOR_CYAN);
        }
    }

    // Time axis ticks (-45s, -30s, -15s, NOW)
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(COLOR_DARKGRAY, COLOR_CARD);
    spr.drawString("-45s", startX + 3 * 10, baseLineY + 4, 1);
    spr.drawString("-30s", startX + 18 * 10, baseLineY + 4, 1);
    spr.drawString("-15s", startX + 33 * 10, baseLineY + 4, 1);

    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    spr.drawString("NOW (0s)", startX + (WAVE_SLOTS - 1) * 10, baseLineY + 4, 1);

    drawFooter();
}

static void drawPageClockWeather() {
    drawHeader("CLOCK & WEATHER", COLOR_CYAN);

    int cardW = (SCREEN_W - 34) / 2;
    int cardH = 126;
    int x1 = 12;
    int x2 = x1 + cardW + 10;
    int y1 = 36;

    // Card 1: Clock & Calendar
    spr.fillRoundRect(x1, y1, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x1, y1, cardW, cardH, 8, COLOR_CARD_BORDER);

    struct tm timeinfo;
    bool timeOk = getLocalTime(&timeinfo, 10);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    spr.drawString("LOCAL TIME (GMT+8)", x1 + 12, y1 + 10, 1);

    if (timeOk) {
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_WHITE, COLOR_CARD);
        spr.drawString(timeStr, x1 + 12, y1 + 28, 6);

        static const char* const WDAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d %s",
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 WDAYS[timeinfo.tm_wday % 7]);
        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
        spr.drawString(dateStr, x1 + 12, y1 + 82, 2);

        spr.setTextColor(COLOR_GREEN, COLOR_CARD);
        spr.drawString("NTP Synchronized", x1 + 12, y1 + 104, 1);
    } else {
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString("Syncing NTP...", x1 + cardW / 2, y1 + 55, 4);
        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
        spr.drawString("Waiting for time server", x1 + cardW / 2, y1 + 95, 2);
    }

    // Card 2: Weather Forecast
    spr.fillRoundRect(x2, y1, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x2, y1, cardW, cardH, 8, COLOR_CARD_BORDER);

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_AMBER, COLOR_CARD);
    extern String weatherCity;
    String wTitle = weatherCity.length() ? weatherCity : "WEATHER FORECAST";
    spr.drawString(wTitle, x2 + 12, y1 + 10, 1);

    if (currentWeather.valid) {
        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_WHITE, COLOR_CARD);
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%.1f", currentWeather.temp);
        spr.drawString(tempStr, x2 + 12, y1 + 28, 6);
        spr.drawString(" C", x2 + 12 + spr.textWidth(tempStr, 6) + 4, y1 + 36, 4);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_CYAN, COLOR_CARD);
        spr.drawString(currentWeather.conditionStr, x2 + 12, y1 + 82, 2);

        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
        char envStr[48];
        snprintf(envStr, sizeof(envStr), "Hum: %d%%  |  Wind: %.0f km/h",
                 currentWeather.humidity, currentWeather.windSpeed);
        spr.drawString(envStr, x2 + 12, y1 + 104, 1);
    } else {
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_CYAN, COLOR_CARD);
        spr.drawString("Updating...", x2 + cardW / 2, y1 + 55, 4);
        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
        spr.drawString("Fetching Open-Meteo", x2 + cardW / 2, y1 + 95, 2);
    }

    // Bottom Mini Shield Ticker (y: 172 to 200)
    spr.fillRoundRect(14, 172, SCREEN_W - 28, 28, 6, COLOR_CARD);
    spr.drawRoundRect(14, 172, SCREEN_W - 28, 28, 6, COLOR_CARD_BORDER);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);

    uint32_t totalQueries = totalBlocked + totalAllowed;
    float blockRate = (totalQueries > 0) ? ((float)totalBlocked * 100.0f / totalQueries) : 0.0f;
    char ticker[80];
    snprintf(ticker, sizeof(ticker), "Shield %s  |  Blocked: %lu (%.1f%%)  |  Allowed: %lu  |  Clients: %d",
             blockingOn ? "ON" : "OFF", totalBlocked, blockRate, totalAllowed, numClients);
    spr.drawString(ticker, 24, 186, 2);

    drawFooter();
}

static void drawPageTopBlocked() {
    drawHeader("TOP BLOCKED DOMAINS", COLOR_RED);

    int boxX = 14;
    int boxY = 36;
    int boxW = SCREEN_W - 28;
    int boxH = 164;

    spr.fillRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_CARD);
    spr.drawRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_CARD_BORDER);

    if (numTopBlocked == 0) {
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_GRAY, COLOR_CARD);
        spr.drawString("No blocked domains recorded yet.", boxX + (boxW / 2), boxY + 70, 2);
        spr.drawString("Traffic will be ranked here in real time.", boxX + (boxW / 2), boxY + 95, 2);
    } else {
        int maxItems = (numTopBlocked > 5) ? 5 : numTopBlocked;
        uint32_t maxCount = topBlocked[0].count;
        if (maxCount == 0) maxCount = 1;

        int rowH = 30;
        int startY = boxY + 8;

        for (int i = 0; i < maxItems; i++) {
            int ry = startY + (i * rowH);

            uint16_t badgeColor = COLOR_GRAY;
            if (i == 0) badgeColor = COLOR_AMBER;
            else if (i == 1) badgeColor = COLOR_CYAN;
            else if (i == 2) badgeColor = 0xFA60;

            spr.fillRoundRect(boxX + 10, ry + 2, 32, 22, 4, badgeColor);
            spr.setTextDatum(MC_DATUM);
            spr.setTextColor(COLOR_BG, badgeColor);
            char rkStr[8];
            snprintf(rkStr, sizeof(rkStr), "#%d", i + 1);
            spr.drawString(rkStr, boxX + 26, ry + 13, 2);

            spr.setTextDatum(ML_DATUM);
            spr.setTextColor(COLOR_WHITE, COLOR_CARD);
            String dName = topBlocked[i].domain;
            if (dName.length() > 32) dName = dName.substring(0, 29) + "...";
            spr.drawString(dName, boxX + 50, ry + 13, 2);

            char cStr[16];
            snprintf(cStr, sizeof(cStr), "%lu hits", topBlocked[i].count);
            spr.setTextDatum(MR_DATUM);
            spr.setTextColor(COLOR_RED, COLOR_CARD);
            spr.drawString(cStr, boxX + boxW - 14, ry + 13, 2);

            int pBarW = 110;
            int fillW = (int)((float)topBlocked[i].count / maxCount * pBarW);
            if (fillW < 2) fillW = 2;
            int pBarX = boxX + boxW - 130 - pBarW;
            spr.drawRoundRect(pBarX, ry + 8, pBarW, 8, 2, COLOR_DARKGRAY);
            spr.fillRoundRect(pBarX, ry + 8, fillW, 8, 2, COLOR_RED);

            if (i < maxItems - 1) {
                spr.drawFastHLine(boxX + 10, ry + rowH - 1, boxW - 20, COLOR_CARD_BORDER);
            }
        }
    }

    drawFooter();
}

static void drawPageSystem() {
    drawHeader("SYSTEM & HARDWARE HEALTH", COLOR_CYAN);

    int cardW = (SCREEN_W - 34) / 2;
    int cardH = 78;
    int x1 = 12;
    int x2 = x1 + cardW + 10;
    int y1 = 36;
    int y2 = y1 + cardH + 10;

    // Card 1: Network & WiFi
    spr.fillRoundRect(x1, y1, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x1, y1, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    spr.drawString("NETWORK INTERFACE", x1 + 10, y1 + 8, 1);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    spr.drawString("IP: " + WiFi.localIP().toString(), x1 + 10, y1 + 24, 2);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("SSID: " + WiFi.SSID() + " (" + String(WiFi.RSSI()) + " dBm)", x1 + 10, y1 + 44, 1);
    char netStatusStr[48];
    if (netIsOnline && netRttMs >= 0) {
        snprintf(netStatusStr, sizeof(netStatusStr), "WAN: Online (%ldms)  |  Port 53", (long)netRttMs);
        spr.setTextColor(COLOR_GREEN, COLOR_CARD);
    } else {
        snprintf(netStatusStr, sizeof(netStatusStr), "WAN: OFFLINE ALERT  |  Port 53");
        spr.setTextColor(COLOR_RED, COLOR_CARD);
    }
    spr.drawString(netStatusStr, x1 + 10, y1 + 58, 1);

    // Card 2: Memory & PSRAM
    spr.fillRoundRect(x2, y1, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x2, y1, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GREEN, COLOR_CARD);
    spr.drawString("ESP32-S3 MEMORY", x2 + 10, y1 + 8, 1);

    uint32_t freeHeap = ESP.getFreeHeap() / 1024;
    uint32_t freePsram = ESP.getFreePsram() / 1024;
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char memStr[48];
    snprintf(memStr, sizeof(memStr), "Heap: %lu KB Free", freeHeap);
    spr.drawString(memStr, x2 + 10, y1 + 24, 2);

    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    char psStr[48];
    snprintf(psStr, sizeof(psStr), "PSRAM: %lu KB / 8192 KB", freePsram);
    spr.drawString(psStr, x2 + 10, y1 + 44, 1);

    // PSRAM visual bar
    spr.drawRoundRect(x2 + 10, y1 + 60, cardW - 20, 6, 2, COLOR_DARKGRAY);
    spr.fillRoundRect(x2 + 10, y1 + 60, (cardW - 20) * freePsram / 8192, 6, 2, COLOR_GREEN);

    // Card 3: SoC & Temperature
    spr.fillRoundRect(x1, y2, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x1, y2, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_AMBER, COLOR_CARD);
    spr.drawString("SOC & TEMPERATURE", x1 + 10, y2 + 8, 1);

    float temp = temperatureRead();
    uint16_t tColor = (temp < 50.0f) ? COLOR_GREEN : (temp < 70.0f ? COLOR_AMBER : COLOR_RED);
    spr.setTextColor(tColor, COLOR_CARD);
    char tempStr[32];
    snprintf(tempStr, sizeof(tempStr), "CPU Temp: %.1f C", temp);
    spr.drawString(tempStr, x1 + 10, y2 + 24, 2);

    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("Dual-Core Xtensa LX7 @ 240MHz", x1 + 10, y2 + 44, 1);
    spr.drawString("Flash: 16 MB QIO  |  AMOLED QSPI", x1 + 10, y2 + 58, 1);

    // Card 4: Flash Storage & Uptime
    spr.fillRoundRect(x2, y2, cardW, cardH, 8, COLOR_CARD);
    spr.drawRoundRect(x2, y2, cardW, cardH, 8, COLOR_CARD_BORDER);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_PURPLE, COLOR_CARD);
    spr.drawString("STORAGE & UPTIME", x2 + 10, y2 + 8, 1);

    uint32_t upSec = millis() / 1000;
    char utStr[48];
    snprintf(utStr, sizeof(utStr), "%lud %02luh %02lum %02lus", upSec / 86400, (upSec % 86400) / 3600, (upSec % 3600) / 60, upSec % 60);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    spr.drawString(utStr, x2 + 10, y2 + 24, 2);

    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    char fsStr[48];
    snprintf(fsStr, sizeof(fsStr), "Rules: %u  |  Local DNS: %d", numHashes, numLocalDns);
    spr.drawString(fsStr, x2 + 10, y2 + 44, 1);
    spr.drawString("IMU: " + String(getIMUName()) + " (Auto-Flip)", x2 + 10, y2 + 58, 1);

    drawFooter();
}

static void drawPageQR() {
    drawHeader("DASHBOARD QUICK CONNECT", COLOR_CYAN);

    // Build URL for QR code
    String url = "http://" + WiFi.localIP().toString();
    if (WiFi.status() != WL_CONNECTED) {
        url = "http://192.168.4.1"; // Default AP portal IP
    }

    // Generate QR Code (Version 3: 29x29 matrix, ECC_LOW)
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, url.c_str());

    // QR dimensions
    int scale = 4; // 29 * 4 = 116 pixels
    int qrW = qrcode.size * scale;
    int qrX = 30;
    int qrY = 48;
    int quietZone = 8;

    // White background card for QR Code
    spr.fillRoundRect(qrX - quietZone, qrY - quietZone, qrW + (quietZone * 2), qrW + (quietZone * 2), 8, COLOR_WHITE);

    // Draw QR Modules
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                spr.fillRect(qrX + (x * scale), qrY + (y * scale), scale, scale, COLOR_BG);
            }
        }
    }

    // Right Side: Info & Controls panel
    int infoX = qrX + qrW + quietZone + 24;
    int infoY = 44;

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN, COLOR_BG);
    spr.drawString("SCAN WITH SMARTPHONE", infoX, infoY, 4);

    spr.setTextColor(COLOR_WHITE, COLOR_BG);
    spr.drawString("Instant Web Dashboard Access", infoX, infoY + 30, 2);

    spr.setTextColor(COLOR_GRAY, COLOR_BG);
    spr.drawString("URL: " + url, infoX, infoY + 54, 2);
    spr.drawString("Hostname: http://c3adblock.local", infoX, infoY + 74, 2);

    // Control Shortcuts Box
    int boxY = infoY + 100;
    spr.fillRoundRect(infoX, boxY, SCREEN_W - infoX - 16, 48, 6, COLOR_CARD);
    spr.drawRoundRect(infoX, boxY, SCREEN_W - infoX - 16, 48, 6, COLOR_CARD_BORDER);

    spr.setTextColor(COLOR_GREEN, COLOR_CARD);
    spr.drawString("[KEY 2] Tap: Pause | 2x: Flip 180 | Hold: Brightness", infoX + 10, boxY + 8, 1);
    spr.setTextColor(COLOR_AMBER, COLOR_CARD);
    spr.drawString("[KEY 1] Tap: Next Page | Hold: Fetch Update", infoX + 10, boxY + 26, 1);

    drawFooter();
}

static void drawToast() {
    if (millis() < toastExpiry && toastMsg.length() > 0) {
        int toastW = 320;
        int toastH = 46;
        int tx = (SCREEN_W - toastW) / 2;
        int ty = 97;

        spr.fillRoundRect(tx, ty, toastW, toastH, 10, 0x1925);
        spr.drawRoundRect(tx, ty, toastW, toastH, 10, COLOR_CYAN);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_WHITE, 0x1925);
        spr.drawString(toastMsg, SCREEN_W / 2, ty + (toastH / 2), 2);
    }
}

