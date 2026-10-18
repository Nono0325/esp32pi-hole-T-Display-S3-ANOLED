#include "display.h"
#include "rm67162.h"
#include "pins_config.h"
#include "imu.h"
#include <TFT_eSPI.h>
#include <OneButton.h>
#include <Wire.h>
#include <qrcode.h>

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
static uint8_t brightnessIndex = 0;
static bool screenAsleep = false;

// Toast Notifications
static String toastMsg = "";
static uint32_t toastExpiry = 0;

// Query History Graph (Rolling 24 time buckets)
#define HIST_SLOTS 24
static uint16_t histBlocked[HIST_SLOTS] = {0};
static uint16_t histAllowed[HIST_SLOTS] = {0};
static uint8_t histIdx = 0;
static uint32_t lastHistRoll = 0;

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

void recordQueryActivity(bool blocked) {
    if (blocked) {
        if (histBlocked[histIdx] < 65535) histBlocked[histIdx]++;
    } else {
        if (histAllowed[histIdx] < 65535) histAllowed[histIdx]++;
    }
}

void displayToast(const char* message, uint32_t durationMs) {
    toastMsg = message;
    toastExpiry = millis() + durationMs;
}

void setDisplayBrightness(uint8_t brightness) {
    lcd_setBrightness(brightness);
}

void nextPage() {
    currentPage = (currentPage + 1) % MAX_PAGES;
}

void setDisplayPage(uint8_t page) {
    if (page < MAX_PAGES) {
        currentPage = page;
        if (screenAsleep) {
            screenAsleep = false;
            lcd_wakeup();
            lcd_setBrightness(brightnessLevels[brightnessIndex]);
        }
    }
}

static void onBtn1Click() {
    if (screenAsleep) {
        screenAsleep = false;
        lcd_wakeup();
        lcd_setBrightness(brightnessLevels[brightnessIndex]);
        return;
    }
    nextPage();
}

static void onBtn1LongPress() {
    if (screenAsleep) return;
    displayToast("UPDATING RULES...", 3000);
    // Background fetch will be picked up or run
    if (updateUrl.length()) {
        fetchBlocklist(updateUrl);
        displayToast(updateStatus.c_str(), 3500);
    } else {
        displayToast("NO UPDATE URL SET", 2500);
    }
}

static void onBtn2Click() {
    if (screenAsleep) {
        screenAsleep = false;
        lcd_wakeup();
        lcd_setBrightness(brightnessLevels[brightnessIndex]);
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
    brightnessIndex = (brightnessIndex + 1) % 4;
    lcd_setBrightness(brightnessLevels[brightnessIndex]);
    char bStr[32];
    snprintf(bStr, sizeof(bStr), "BRIGHTNESS: %u%%", (brightnessLevels[brightnessIndex] * 100) / 255);
    displayToast(bStr, 2000);
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

    // 6. Draw Boot Splash Screen
    spr.fillSprite(COLOR_BG);
    spr.setTextDatum(MC_DATUM);

    // Glowing logo shield
    spr.fillRoundRect(SCREEN_W / 2 - 130, 40, 260, 90, 16, COLOR_CARD);
    spr.drawRoundRect(SCREEN_W / 2 - 130, 40, 260, 90, 16, COLOR_CYAN);
    spr.setTextColor(COLOR_CYAN, COLOR_CARD);
    spr.drawString("PI-HOLE S3", SCREEN_W / 2, 70, 4);
    spr.setTextColor(COLOR_GREEN, COLOR_CARD);
    spr.drawString("AMOLED EDITION", SCREEN_W / 2, 102, 2);

    spr.setTextColor(COLOR_GRAY, COLOR_BG);
    spr.drawString("Starting DNS Sinkhole & Web Server...", SCREEN_W / 2, 165, 2);
    spr.drawString("LilyGO T-Display-S3 AMOLED", SCREEN_W / 2, 195, 2);

    lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
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
    // Tick buttons, touch and gravity IMU
    btn1.tick();
    btn2.tick();
    checkTouch();
    updateIMU();

    // Roll query history slot every 10 seconds
    if (millis() - lastHistRoll >= 10000UL) {
        lastHistRoll = millis();
        histIdx = (histIdx + 1) % HIST_SLOTS;
        histBlocked[histIdx] = 0;
        histAllowed[histIdx] = 0;
    }

    static uint32_t lastRender = 0;
    if (!force && millis() - lastRender < 350) return; // ~3 FPS smooth dashboard refresh
    lastRender = millis();

    if (screenAsleep) return;

    spr.fillSprite(COLOR_BG);

    switch (currentPage) {
        case 0: drawPageOverview();     break;
        case 1: drawPageActivity();     break;
        case 2: drawPageClockWeather(); break;
        case 3: drawPageTopBlocked();   break;
        case 4: drawPageSystem();       break;
        case 5: drawPageQR();           break;
    }

    drawToast();
    lcd_PushColors(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)spr.getPointer());
}

static void drawHeader(const char* title, uint16_t titleColor) {
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(titleColor, COLOR_BG);
    spr.drawString(title, 12, 6, 2);

    // WiFi Info & IP
    spr.setTextDatum(TR_DATUM);
    String netInfo = "";
    if (WiFi.status() == WL_CONNECTED) {
        netInfo = WiFi.SSID() + " (" + String(WiFi.RSSI()) + "dBm)  " + WiFi.localIP().toString();
    } else {
        netInfo = "WiFi: Disconnected";
    }
    spr.setTextColor(COLOR_GRAY, COLOR_BG);
    spr.drawString(netInfo, SCREEN_W - 12, 6, 2);

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
        int dotX = SCREEN_W - 95 + (i * 14);
        if (i == currentPage) {
            spr.fillCircle(dotX, 222, 4, COLOR_CYAN);
        } else {
            spr.drawCircle(dotX, 222, 3, COLOR_DARKGRAY);
        }
    }
}

static void drawPageOverview() {
    drawHeader("SHIELD OVERVIEW", COLOR_CYAN);

    // ---- Status Banner (y: 34 to 68) ----
    uint16_t statusBg = blockingOn ? 0x0284 : 0x31E0;
    uint16_t statusText = blockingOn ? COLOR_GREEN : COLOR_AMBER;
    spr.fillRoundRect(12, 34, 230, 32, 8, statusBg);
    spr.drawRoundRect(12, 34, 230, 32, 8, statusText);

    spr.fillCircle(28, 50, 5, statusText);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(statusText, statusBg);
    if (blockingOn) {
        spr.drawString("SHIELD ACTIVE", 42, 50, 2);
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
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("Upstream: Quad9 9.9.9.9", SCREEN_W - 24, 50, 2);

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
    spr.drawString("Active LAN Devices", cardX3 + 12, cardY + 76, 2);
    char upStr[32];
    snprintf(upStr, sizeof(upStr), "Uptime: %luh", (millis() / 3600000UL));
    spr.drawString(upStr, cardX3 + 12, cardY + 96, 1);

    drawFooter();
}

static void drawPageActivity() {
    drawHeader("REAL-TIME DNS ACTIVITY", COLOR_CYAN);

    // Graph Area Box (y: 36 to 166, h: 130)
    int graphX = 14;
    int graphY = 36;
    int graphW = SCREEN_W - 28;
    int graphH = 128;

    spr.fillRoundRect(graphX, graphY, graphW, graphH, 10, COLOR_CARD);
    spr.drawRoundRect(graphX, graphY, graphW, graphH, 10, COLOR_CARD_BORDER);

    // Title inside graph
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    spr.drawString("TRAFFIC HISTOGRAM (LAST 4 MINUTES)", graphX + 12, graphY + 10, 1);

    // Legend
    spr.fillCircle(graphX + graphW - 130, graphY + 14, 4, COLOR_RED);
    spr.drawString("Blocked", graphX + graphW - 120, graphY + 10, 1);
    spr.fillCircle(graphX + graphW - 65, graphY + 14, 4, COLOR_GREEN);
    spr.drawString("Allowed", graphX + graphW - 55, graphY + 10, 1);

    // Find maximum count for scaling
    uint16_t maxCount = 10;
    for (int i = 0; i < HIST_SLOTS; i++) {
        uint16_t tot = histBlocked[i] + histAllowed[i];
        if (tot > maxCount) maxCount = tot;
    }

    // Grid baseline
    int baseLineY = graphY + graphH - 22;
    spr.drawFastHLine(graphX + 10, baseLineY, graphW - 20, COLOR_DARKGRAY);

    // Draw Histogram Bars
    int barSlotW = (graphW - 30) / HIST_SLOTS;
    int barW = barSlotW - 4;
    int maxBarH = graphH - 52;

    for (int i = 0; i < HIST_SLOTS; i++) {
        int slot = (histIdx + 1 + i) % HIST_SLOTS;
        uint16_t blk = histBlocked[slot];
        uint16_t alw = histAllowed[slot];
        uint16_t tot = blk + alw;

        int totalH = (int)((float)tot / maxCount * maxBarH);
        int blockedH = (int)((float)blk / maxCount * maxBarH);
        int allowedH = totalH - blockedH;

        int bx = graphX + 16 + (i * barSlotW);

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
            // Idle dot
            spr.drawFastHLine(bx, baseLineY, barW, COLOR_DARKGRAY);
        }
    }

    // Scale label
    spr.setTextDatum(BR_DATUM);
    spr.setTextColor(COLOR_GRAY, COLOR_CARD);
    char scaleStr[16];
    snprintf(scaleStr, sizeof(scaleStr), "Max %uq", maxCount);
    spr.drawString(scaleStr, graphX + graphW - 12, baseLineY - 2, 1);

    // Bottom Summary Banner (y: 172 to 200)
    spr.fillRoundRect(14, 172, SCREEN_W - 28, 28, 6, COLOR_CARD);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(COLOR_WHITE, COLOR_CARD);
    char statSummary[64];
    snprintf(statSummary, sizeof(statSummary), "Total: %lu queries   |   Blocked: %lu   |   Allowed: %lu",
             totalBlocked + totalAllowed, totalBlocked, totalAllowed);
    spr.drawString(statSummary, 24, 186, 2);

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
    spr.drawString("DNS Port: 53  |  Dashboard: 80", x1 + 10, y1 + 58, 1);

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
    snprintf(fsStr, sizeof(fsStr), "Blocklist DB: %u domains", numHashes);
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
