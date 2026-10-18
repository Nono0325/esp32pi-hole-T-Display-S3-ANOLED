# 🌟 T-Display-S3 AMOLED AdBlocker (DNS 阻擋器 & 儀表板)

本專案由 [M-Abozaid/esp32-c3-adblock](https://github.com/M-Abozaid/esp32-c3-adblock) 成功移植並全面升級為適用於 **LilyGO T-Display-S3 AMOLED** 的高階硬體專案！

充分發揮了 ESP32-S3 雙核心 240MHz、16MB 超大 Flash、8MB OPI PSRAM，以及 1.91 吋 536×240 高對比度 RM67162 AMOLED 螢幕的特性。支援雙核獨立運作、雙緩衝全幀顯示、即時流量柱狀圖、QR Code 手機直連，以及實體按鍵與觸控即時控制！

---

## 🚀 主要特色與硬體升級

| 特性 | 原專案 (ESP32-C3) | 本專案 (T-Display-S3 AMOLED) |
| :--- | :--- | :--- |
| **處理器晶片** | 單核心 RISC-V 160MHz | **雙核心 Xtensa LX7 @ 240MHz** |
| **記憶體 (SRAM / PSRAM)** | 400KB SRAM (無 PSRAM) | **512KB SRAM + 8MB 高速 OPI PSRAM** |
| **Flash 容量與分割區** | 4MB Flash (規則庫限制約 25 萬筆) | **16MB Flash (LittleFS 擴充至 10MB，支援破百萬規則)** |
| **螢幕支援** | 無螢幕，僅純 Web 介面 | **1.91 吋 536×240 RM67162 QSPI AMOLED 螢幕** |
| **雙緩衝圖形引擎** | 無 | **TFT_eSprite 雙緩衝 (~5ms 刷新，零延遲不干擾 DNS)** |
| **實體控制與互動** | 僅 BOOT 鍵重置 WiFi | **雙硬體按鍵 + CST816S 觸控手勢 + 即時 Toast 彈窗** |
| **手機連線** | 需手動輸入 IP 或 mDNS | **螢幕自動產生專屬 QR Code，相機一掃秒開儀表板** |

---

## 🖥️ 螢幕即時資訊與 4 大儀表板頁面

按壓 **按鍵 1 (GPIO 0)** 即可在以下 4 個頁面間無縫切換：

### 1. 🛡️ 總覽儀表板 (Page 0: Protection Overview)
- **頂部狀態列**:
  - `[🛡️ PI-HOLE S3]` 系統標誌
  - WiFi SSID、即時訊號強度 (如 `-52 dBm`) 與本機 IP 位址 (如 `192.168.1.120`)
- **防護狀態指示膠囊**:
  - 運行中：綠色微光膠囊 `[ ● SHIELD ACTIVE ]`
  - 暫停中：橘黃色動態倒數膠囊 `[ ⏸ PAUSED 04:32 ]` (精確秒數即時遞減)
  - 規則庫容量：`Rules: 254k domains`，上游 DNS：`Quad9 9.9.9.9`
- **三大科技感數據卡片 (Cards)**:
  - **攔截廣告卡 (紅/粉霓虹)**: 總阻擋次數 (如 `12,845`) + 阻擋率百分比標籤 (如 `28.6% RATE`)。
  - **總查詢卡 (賽博青/藍)**: 總查詢數 (如 `45,210`) + 允許數 + **雙色比例進度條** (紅綠分段動態條)。
  - **連線設備卡 (極光紫)**: 當前區域網路活躍設備數 (如 `14 DEVICES`) + 系統已運作時間 (Uptime)。

### 2. 📊 即時流量柱狀圖 (Page 1: Real-time Traffic Graph)
- **動態長條圖 (24 時段滾動更新)**:
  - 記錄過去 4 分鐘每 10 秒區間的查詢與攔截流量。
  - 長條圖採 **紅綠雙色堆疊** (綠色代表允許查詢、紅色代表攔截廣告)，網路流量與阻擋高峰一目了然！
- **底部統計摘要**: 即時呈現總查詢數、攔截數與放行數。

### 3. ⚙️ 系統與硬體監控 (Page 2: Hardware Health)
- **網路介面卡**: 顯示 IP、MAC 位址、SSID 與訊號、DNS Port (53) 與 Web Port (80)。
- **記憶體卡**: 內部 Heap 與 **8MB PSRAM** 即時剩餘容量與圖形化佔用進度條。
- **SoC 與溫度卡**: 雙核心 240MHz 資訊，以及 **即時 CPU 晶片溫度** (低於 50°C 顯示綠色，高於 50°C 顯示黃色，過熱顯示紅色)。
- **儲存與運行卡**: 完整系統運作天數時分秒、LittleFS 規則資料庫大小與最新更新狀態。

### 4. 📱 快速連線 QR Code 與控制中心 (Page 3: Quick Connect)
- **大尺寸高對比 QR Code**:
  - 自動將目前的 Web 儀表板網址 (如 `http://192.168.1.120/`) 編碼為 QR Code。
  - 手機拿起相機直接掃描，一秒開啟手機版控制後台！
- **快捷控制說明**:
  - 顯示預設帳號密碼 (`admin` / `admin123`)。
  - 顯示目前螢幕亮度與快捷按鍵提示。

### 💬 即時浮動彈窗 (Toast Notification)
在設備上按鍵操作（切換暫停、調整亮度、更新規則庫），或在手機/電腦網頁後台點擊操作時，AMOLED 螢幕中央會立即彈出懸浮半透明膠囊通知（如 `PAUSED (5 MINS)`、`SHIELD RESUMED`、`BRIGHTNESS: 60%`、`UPDATING RULES...`），提供極佳的實體操作回饋！

---

## 🎮 實體操作與控制方式

### 實體按鍵功能 (板載 2 顆按鍵)
1. **按鍵 1 (PIN_BUTTON_1 / GPIO 0，即 BOOT 鍵)**:
   - **單擊 (Short Click)**: 切換頁面 (總覽 -> 流量圖 -> 系統監控 -> QR Code)。
   - **長按 (Long Press > 1.5 秒)**: 手動觸發遠端規則庫即時更新 (`fetchBlocklist`)。
   - **開機時按住不放**: 重置 WiFi 設定，開機自動進入配網熱點 (Captive Portal)。
2. **按鍵 2 (PIN_BUTTON_2 / GPIO 21，即 KEY 鍵)**:
   - **單擊 (Short Click)**: 即時切換 AdBlock 狀態 (運行中 ⇄ 暫停 5 分鐘 ⇄ 立即恢復)。
   - **雙擊 (Double Click)**: **手動 180 度翻轉螢幕** (`正常橫向 0°` ⇄ `倒置橫向 180°`)，方便不同方向插線擺放。
   - **長按 (Long Press > 1.5 秒)**: 循環切換螢幕亮度 (`100%` ➔ `66%` ➔ `30%` ➔ `螢幕休眠省電` ➔ `喚醒`)。

### 🔄 重力感測自動翻轉 (IMU Auto-Rotation)
- **多晶片自動探測**：程式開機自動掃描 I2C 匯流排（SDA=3, SCL=2），支援常見重力加速度感測器：
  - **MPU6050 / MPU6500 / MPU9250** (`0x68` / `0x69`)
  - **QMA6100P / QMA7981** (`0x12`)
  - **LIS3DH / LIS2DH** (`0x18` / `0x19`)
  - **BMA423 / BMA400** (`0x18` / `0x19`)
  - **LSM6DS3 / LSM6DSL** (`0x6A` / `0x6B`)
  - **ADXL345** (`0x53`)
- **智慧濾波防抖**：當設備翻轉時，自動依據重力加速度翻轉螢幕 180 度，並具有 360ms 防抖遲滯保護，避免拿取時誤觸。
- **三種翻轉途徑**：
  1. **重力自動翻轉**：若接上 IMU 感測器自動感應翻轉。
  2. **實體按鍵雙擊**：隨時連按兩下板載按鍵 2 即可手動翻轉 180 度。
  3. **網頁端一鍵翻轉**：網頁儀表板上方設有「`🔄 Flip 180°`」按鈕，手機/電腦點擊即秒翻轉！

### 觸控支援 (若為 Touch 觸控版本)
- 內建自動探測 CST816S 觸控晶片。
- **向左/向右滑動**: 切換上一頁 / 下一頁。
- **點擊螢幕中央**: 即時切換暫停 / 恢復防護。

---

## 📂 專案檔案結構

```
c:\Nono\esp32pi-hole/
├── boards/
│   └── lilygo-t-amoled.json    # LilyGO T-Display-S3 AMOLED 板卡硬體定義
├── lib/
│   ├── OneButton/              # 實體按鍵多功能驅動 (單擊、長按)
│   ├── TFT_eSPI/               # 圖形與雙緩衝 Sprite 引擎
│   └── QRCode/                 # 輕量級 QR Code 生成器
├── src/
│   ├── display.h               # AMOLED 螢幕介面定義
│   ├── display.cpp             # 4 大儀表板頁面、動態圖表與觸控繪製邏輯
│   ├── pins_config.h           # T-Display-S3 AMOLED QSPI 與週邊腳位配置
│   ├── rm67162.h               # RM67162 QSPI AMOLED 驅動標頭檔
│   ├── rm67162.cpp             # 高速 QSPI (75MHz) DMA 通訊與硬體亮度控制
│   ├── secrets.h               # WiFi 與 Web 登入憑證 (已預設建立)
│   ├── secrets.example.h       # 憑證範本
│   ├── page.h                  # Web Dashboard 網頁 HTML/JS
│   └── main.cpp                # 核心 DNS 轉發、統計分析、Web 伺服器與主迴圈
├── tools/
│   ├── build_blocklist.py      # Python 阻擋規則庫編譯器 (轉為 40-bit FNV-1a Hash)
│   └── test_build_blocklist.py # 單元測試
├── partitions_16MB.csv         # 16MB Flash 分割表 (3MB OTA + 10MB LittleFS)
└── platformio.ini              # PlatformIO 設定 (預設 env:t-display-s3-amoled)
```

---

## 🛠️ 編譯與燒錄指南

### 方法一：使用 VS Code + PlatformIO (推薦)
1. 在 VS Code 中點擊 `File` ➔ `Open Folder`，選擇 `c:\Nono\esp32pi-hole`。
2. PlatformIO 會自動識別專案並載入 `platformio.ini`。
3. 連接 LilyGO T-Display-S3 AMOLED 的 USB-C 傳輸線至電腦。
4. 點擊底部狀態列的 **PlatformIO: Build** (打勾符號) 編譯。
5. 點擊 **PlatformIO: Upload** (箭頭符號) 燒錄韌體至開發板。
6. 打開 Serial Monitor (鮑率 `115200`)，即可看到啟動日誌與連線 IP。

> **提示**：若上傳時抓不到 COM 埠，請按住板子上的 **BOOT 鍵**，點一下 **RST 鍵**，再放開 **BOOT 鍵**，板子即進入燒錄下載模式。

---

### 方法二：產生並上傳阻擋規則庫 (Blocklist)

原專案採用極致效能的 40-bit 雜湊演算法，可將幾十萬筆網域名稱壓縮至數 MB 內，在 ESP32 上二分搜尋秒級比對：

1. 執行 Python 腳本抓取並打包最新規則（預設收錄 StevenBlack + Hagezi Light 約 10 萬筆廣告惡意網址）：
   ```powershell
   python tools/build_blocklist.py data/blocklist.bin
   ```
2. **上傳規則庫至 ESP32**:
   - **方式 A (免插線網頁上傳，最方便)**:
     - 在手機或電腦瀏覽器打開 `http://<ESP32-IP>/` (或掃描螢幕第 4 頁的 QR Code)。
     - 在「BLOCKLIST — UPLOAD」區段選取剛剛產生的 `blocklist.bin`，點擊 Upload 即秒更新！
   - **方式 B (定時遠端自動更新)**:
     - 在網頁「BLOCKLIST — REMOTE AUTO-UPDATE」填入預先託管的 `blocklist.bin` 下載網址（如 GitHub Release），ESP32 將每 24 小時自動抓取最新名單！
