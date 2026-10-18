#include "spectrum.h"
#include <WiFi.h>

static SpectrumData specData;
static uint32_t lastAutoScan = 0;

void initSpectrumRadar() {
    memset(&specData, 0, sizeof(specData));
    specData.bestChannel = 1;
    specData.currentChannel = WiFi.channel() ? WiFi.channel() : 1;
    specData.scanning = false;
    specData.lastScanMs = 0;
    // Initial scan triggered after boot
    triggerSpectrumScan();
}

bool triggerSpectrumScan() {
    if (specData.scanning) return false;
    int16_t status = WiFi.scanComplete();
    if (status == -1) return false; // Already scanning

    Serial.println("[spectrum] Starting 2.4GHz WiFi spectrum scan (async)...");
    WiFi.scanNetworks(true, true); // Non-blocking background scan
    specData.scanning = true;
    return true;
}

void updateSpectrumScan() {
    // Animate radar beam sweep
    specData.sweepAngle = (specData.sweepAngle + 6) % 360;

    if (!specData.scanning) {
        // Optional auto-refresh scan every 60 seconds if in radar mode
        return;
    }

    int16_t n = WiFi.scanComplete();
    if (n == -1) {
        // Scan still in progress
        return;
    }

    specData.scanning = false;
    specData.lastScanMs = millis();

    if (n < 0) {
        // Scan error
        Serial.printf("[spectrum] Scan error: %d\n", n);
        WiFi.scanDelete();
        return;
    }

    Serial.printf("[spectrum] Scan complete! Found %d access points on 2.4GHz.\n", n);

    // Reset counters
    memset(specData.chCount, 0, sizeof(specData.chCount));
    memset(specData.chScore, 0, sizeof(specData.chScore));
    specData.currentChannel = WiFi.channel() ? WiFi.channel() : 1;
    String curSsid = WiFi.SSID();

    uint8_t count = (n > MAX_SPECTRUM_APS) ? MAX_SPECTRUM_APS : (uint8_t)n;
    specData.apCount = count;

    for (int i = 0; i < count; i++) {
        String ssid = WiFi.SSID(i);
        strncpy(specData.aps[i].ssid, ssid.c_str(), sizeof(specData.aps[i].ssid) - 1);
        specData.aps[i].ssid[sizeof(specData.aps[i].ssid) - 1] = '\0';
        if (strlen(specData.aps[i].ssid) == 0) {
            strcpy(specData.aps[i].ssid, "<Hidden SSID>");
        }

        specData.aps[i].rssi = (int8_t)WiFi.RSSI(i);
        uint8_t ch = (uint8_t)WiFi.channel(i);
        specData.aps[i].channel = (ch >= 1 && ch <= 14) ? ch : 1;
        specData.aps[i].encType = (uint8_t)WiFi.encryptionType(i);

        const uint8_t* b = WiFi.BSSID(i);
        if (b) {
            memcpy(specData.aps[i].bssid, b, 6);
        } else {
            memset(specData.aps[i].bssid, 0, 6);
        }

        specData.aps[i].isCurrent = (ssid == curSsid && specData.aps[i].channel == specData.currentChannel);

        if (specData.aps[i].channel >= 1 && specData.aps[i].channel <= 14) {
            specData.chCount[specData.aps[i].channel]++;
        }
    }

    // Sort APs by RSSI descending (strongest signals first)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (specData.aps[j].rssi > specData.aps[i].rssi) {
                WifiApInfo tmp = specData.aps[i];
                specData.aps[i] = specData.aps[j];
                specData.aps[j] = tmp;
            }
        }
    }

    // Calculate 20MHz RF channel overlap and interference scores (Channels 1 to 14)
    float rawScore[15] = {0.0f};
    float maxScore = 1.0f;

    for (int c = 1; c <= 14; c++) {
        for (int i = 0; i < count; i++) {
            int dist = abs(c - (int)specData.aps[i].channel);
            if (dist <= 4) {
                // Signal weight: higher RSSI means much more interference
                float sigWeight = (float)(specData.aps[i].rssi + 105);
                if (sigWeight < 5.0f) sigWeight = 5.0f;
                // Adjacent channel decay factor
                float overlapFactor = (float)(5 - dist) / 5.0f;
                rawScore[c] += (sigWeight * overlapFactor);
            }
        }
        if (rawScore[c] > maxScore) maxScore = rawScore[c];
    }

    // Normalize channel congestion scores to 0 ~ 100%
    for (int c = 1; c <= 14; c++) {
        specData.chScore[c] = (uint8_t)((rawScore[c] * 100.0f) / maxScore);
        if (specData.chScore[c] > 100) specData.chScore[c] = 100;
        if (specData.chCount[c] == 0 && rawScore[c] < 15.0f) {
            specData.chScore[c] = (uint8_t)(rawScore[c] * 1.5f);
        }
    }

    // Pick best channel among non-overlapping 2.4GHz channels: 1, 6, 11 (and 13)
    uint8_t candidates[] = {1, 6, 11, 13};
    uint8_t best = 1;
    float minPenalty = 999999.0f;

    for (uint8_t ch : candidates) {
        if (rawScore[ch] < minPenalty) {
            minPenalty = rawScore[ch];
            best = ch;
        }
    }
    specData.bestChannel = best;

    Serial.printf("[spectrum] Current Ch: %d | Recommended Best Ch: %d (Score: %d%%)\n",
                  specData.currentChannel, specData.bestChannel, specData.chScore[best]);

    WiFi.scanDelete(); // Release memory consumed by scan
}

bool isSpectrumScanning() {
    return specData.scanning;
}

const SpectrumData& getSpectrumData() {
    return specData;
}

static String jsonEscape(const char* s) {
    String out = "";
    while (*s) {
        if (*s == '"') out += "\\\"";
        else if (*s == '\\') out += "\\\\";
        else if (*s == '\n') out += "\\n";
        else if (*s == '\r') out += "\\r";
        else if (*s == '\t') out += "\\t";
        else if ((uint8_t)*s < 0x20) {
            // skip control characters
        } else {
            out += *s;
        }
        s++;
    }
    return out;
}

String getSpectrumJson() {
    String j = "{";
    j += "\"scanning\":" + String(specData.scanning ? "true" : "false");
    j += ",\"lastScan\":" + String(specData.lastScanMs);
    j += ",\"curCh\":" + String(specData.currentChannel);
    j += ",\"bestCh\":" + String(specData.bestChannel);
    j += ",\"apCount\":" + String(specData.apCount);

    // Channel Congestion & AP Count array
    j += ",\"channels\":[";
    for (int c = 1; c <= 14; c++) {
        if (c > 1) j += ",";
        j += "{\"ch\":" + String(c) +
             ",\"count\":" + String(specData.chCount[c]) +
             ",\"score\":" + String(specData.chScore[c]) + "}";
    }
    j += "]";

    // AP List
    j += ",\"aps\":[";
    for (int i = 0; i < specData.apCount; i++) {
        if (i > 0) j += ",";
        char bssidStr[18];
        snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 specData.aps[i].bssid[0], specData.aps[i].bssid[1],
                 specData.aps[i].bssid[2], specData.aps[i].bssid[3],
                 specData.aps[i].bssid[4], specData.aps[i].bssid[5]);

        j += "{\"ssid\":\"" + jsonEscape(specData.aps[i].ssid) + "\"" +
             ",\"rssi\":" + String(specData.aps[i].rssi) +
             ",\"ch\":" + String(specData.aps[i].channel) +
             ",\"bssid\":\"" + String(bssidStr) + "\"" +
             ",\"enc\":" + String(specData.aps[i].encType) +
             ",\"isCur\":" + (specData.aps[i].isCurrent ? "true" : "false") + "}";
    }
    j += "]}";
    return j;
}
