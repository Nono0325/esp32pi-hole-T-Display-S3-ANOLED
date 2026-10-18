#pragma once
// Dashboard & Settings HTML for the T-Display-S3 AMOLED AdBlocker web UI.
// Built for rock-solid stability with zero layout shift, in-place DOM updates,
// tabular numbers, fixed column layouts, and smooth real-time refreshes.
// Features a 2-Tab Navigation: [📊 儀表板總覽] and [⚙️ 系統設定專區].

const char PAGE[] PROGMEM = R"HTML(<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>T-Display-S3 AMOLED 廣告攔截管理系統</title><style>
*{box-sizing:border-box}
html,body{font:14px system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI","PingFang TC","Microsoft JhengHei",Roboto,sans-serif;margin:0;background:#0d1117;color:#c9d1d9;overflow-y:scroll}
header{background:#161b22;padding:14px 18px;border-bottom:1px solid #30363d;display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px}
h1{margin:0;font-size:18px}h1 span{color:#3fb950}.wrap{padding:16px;max-width:1050px;margin:auto}
.nav-tabs{display:flex;gap:6px;background:#161b22;padding:6px;border-radius:8px;border:1px solid #30363d;margin-bottom:16px}
.nav-tab{flex:1;text-align:center;padding:10px 16px;border-radius:6px;border:0;background:transparent;color:#8b949e;font-size:14px;font-weight:600;cursor:pointer;transition:all 0.15s}
.nav-tab:hover{color:#c9d1d9;background:#21262d}
.nav-tab.active{background:#238636;color:#ffffff}
.tab-content{display:none}
.tab-content.active{display:block}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(125px,1fr));gap:12px;margin-bottom:16px}
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:12px 14px;min-height:70px}
.card .v{font-size:20px;font-weight:600;font-variant-numeric:tabular-nums;line-height:1.2}
.card .l{color:#8b949e;font-size:12px;margin-top:4px}
table{width:100%;border-collapse:collapse;background:#161b22;border-radius:8px;overflow:hidden;margin-bottom:18px;table-layout:fixed}
th,td{padding:8px 12px;text-align:left;border-bottom:1px solid #21262d;font-size:13px;font-variant-numeric:tabular-nums;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
th{background:#21262d;color:#8b949e}tr:hover td{background:#1c2128}
.b{color:#f85149}.a{color:#3fb950}
.tag{background:#30363d;border-radius:4px;padding:2px 6px;font-size:11px}
.badge-blk{background:#3b1d1d;color:#f85149;border:1px solid #da3633;border-radius:4px;padding:2px 6px;font-size:11px;font-weight:600}
.badge-alw{background:#1b3320;color:#3fb950;border:1px solid #2ea043;border-radius:4px;padding:2px 6px;font-size:11px;font-weight:600}
.badge-local{background:#0d3846;color:#58a6ff;border:1px solid #1f6feb;border-radius:4px;padding:2px 6px;font-size:11px;font-weight:600}
button{background:#21262d;color:#c9d1d9;border:1px solid #30363d;border-radius:5px;padding:6px 12px;cursor:pointer;font-size:13px}
button:hover{background:#30363d;border-color:#8b949e}.ban{color:#f85149}
input,select{background:#0d1117;border:1px solid #30363d;color:#c9d1d9;border-radius:5px;padding:7px 9px}
input:focus,select:focus{border-color:#58a6ff;outline:none}
h2{font-size:14px;color:#8b949e;margin:20px 0 8px;display:flex;align-items:center;gap:6px}
#blockbar{display:flex;align-items:center;gap:12px;margin-bottom:14px;padding:12px 14px;background:#161b22;border:1px solid #30363d;border-radius:8px;min-height:54px;flex-wrap:wrap}
.panel{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:16px;margin-bottom:18px}
.btn-grp{display:flex;flex-wrap:wrap;gap:8px}
.tool-row{display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:12px}
.section-desc{color:#8b949e;font-size:12px;margin-top:4px;margin-bottom:10px}
</style></head><body>
<header>
  <div style="display:flex;align-items:center;gap:12px">
    <div style="display:inline-flex;align-items:center;justify-content:center;width:36px;height:36px;background:linear-gradient(135deg,#081b29,#1f1035);border:1px solid #00f0ff;border-radius:8px;box-shadow:0 0 12px rgba(0,240,255,0.35);font-size:18px">🛡️</div>
    <div>
      <h1 style="display:flex;align-items:center;gap:8px;font-size:17px">
        <span style="color:#00f0ff;font-weight:800;letter-spacing:0.5px">NonoWuW</span>
        <span style="color:#ffffff;font-size:15px;font-weight:600">CYBER SHIELD</span>
        <span style="background:rgba(216,31,255,0.18);color:#d81fff;border:1px solid rgba(216,31,255,0.45);border-radius:4px;padding:1px 6px;font-size:10px;font-weight:700">( ✧ WuW ✧ )</span>
        <span id=host style="font-size:12px;color:#8b949e;font-weight:normal"></span>
      </h1>
    </div>
  </div>
  <div id=wthr style="font-size:13px;color:#8b949e">🌤️ 台北: 讀取中...</div>
</header>
<div class=wrap>
<div id=credwarn style="display:none;background:#3b1d1d;border:1px solid #f85149;color:#ffb3ae;border-radius:8px;padding:10px 14px;margin-bottom:14px;font-size:13px">
⚠️ <b>目前仍在使用預設管理員密碼。</b>請至「⚙️ 系統設定專區」變更您的專屬密碼。
</div>

<nav class="nav-tabs">
  <button id="tab-dash" class="nav-tab active" onclick="switchTab('dash')">📊 儀表板總覽</button>
  <button id="tab-settings" class="nav-tab" onclick="switchTab('settings')">⚙️ 系統設定專區</button>
</nav>

<!-- ==================== 1. 儀表板總覽分頁 ==================== -->
<div id="view-dash" class="tab-content active">
  <div id=wanalert style="display:none;background:#3d1618;border:1px solid #da3633;color:#f85149;padding:10px 16px;border-radius:8px;margin-bottom:14px;font-weight:600;align-items:center;gap:10px">
    <span style="font-size:20px;line-height:1">⚠️</span>
    <span style="flex:1">網際網路連線中斷警報：無法與上游外網 DNS 伺服器通訊，外網可能已斷線！</span>
    <span id=offline_timer style="font-size:12px;color:#c9d1d9"></span>
  </div>

  <div id=blockbar>
    <span id=blockdot style="font-size:20px;line-height:1">🛡️</span><b id=blockstate style="flex:1" data-on=1>廣告攔截防護中</b>
    <select id=pausedur>
      <option value=30>暫停 30 秒</option>
      <option value=300 selected>暫停 5 分鐘</option>
      <option value=1800>暫停 30 分鐘</option>
      <option value=0>直到手動恢復</option>
    </select>
    <button id=pausebtn onclick=togglePause()>暫停攔截</button>
    <button id=flipbtn onclick=flipScreen() title="旋轉螢幕畫面 180°">🔄 旋轉螢幕 180°</button>
  </div>

  <div class=cards id=sys>
    <div class=card><div class="v b" id=v_blocked>0</div><div class=l>已攔截廣告</div></div>
    <div class=card><div class="v a" id=v_allowed>0</div><div class=l>已允許查詢</div></div>
    <div class=card><div class=v id=v_rate>0.0%</div><div class=l>廣告攔截率</div></div>
    <div class=card><div class=v id=v_qps style="color:#00ffff">0.0 <span style="font-size:12px;color:#8b949e">q/s</span></div><div class=l>即時查詢速率</div></div>
    <div class=card><div class=v id=v_bps style="color:#3fb950">0.0 <span style="font-size:12px;color:#8b949e">KB/s</span></div><div class=l>即時傳輸頻寬</div></div>
    <div class=card><div class=v id=v_peak style="color:#e3b341">0.0 <span style="font-size:12px;color:#8b949e">q/s</span></div><div class=l>歷史峰值速率</div></div>
    <div class=card><div class=v id=v_ping style="color:#58a6ff">-- ms</div><div class=l>外網連線延遲</div></div>
    <div class=card><div class=v id=v_domains>0</div><div class=l>規則庫網域數</div></div>
    <div class=card><div class=v id=v_clients>0</div><div class=l>已連線設備</div></div>
    <div class=card><div class=v id=v_wifi>--</div><div class=l>WiFi 訊號</div></div>
    <div class=card><div class=v id=v_temp>--</div><div class=l>晶片溫度</div></div>
    <div class=card><div class=v id=v_heap>--</div><div class=l>可用 RAM</div></div>
    <div class=card><div class=v id=v_psram>--</div><div class=l>可用 PSRAM</div></div>
    <div class=card><div class=v id=v_uptime>--</div><div class=l>運行時間</div></div>
  </div>

  <h2>🔍 網域解析與攔截診斷工具</h2>
  <div class=panel>
    <div class=tool-row style="margin-bottom:6px">
      <input id=testdom placeholder="輸入欲測試的網域，例如 adservice.google.com 或 google.com" style="flex:1;min-width:240px">
      <button onclick="runDnsTest()" style="background:#1f6feb;color:#fff;border-color:#388bfd">診斷測試</button>
      <button onclick="flushDnsCache()" title="清除內部記憶體 DNS 快取">🧹 清空快取</button>
    </div>
    <div id=testres style="font-size:13px;min-height:18px;color:#8b949e"></div>
  </div>

  <h2>📊 即時 DNS 查詢日誌 (Live Query Log)</h2>
  <table id=lt>
    <colgroup><col style="width:12%"><col style="width:20%"><col style="width:46%"><col style="width:12%"><col style="width:10%"></colgroup>
    <thead><tr><th>時間</th><th>設備 IP</th><th>查詢網域</th><th>處理狀態</th><th>快捷動作</th></tr></thead>
    <tbody><tr><td colspan=5 style="color:#8b949e">等待網路查詢中...</td></tr></tbody>
  </table>

  <h2>🏆 熱門攔截排行榜 (Top Blocked)</h2>
  <table id=tbt>
    <colgroup><col style="width:12%"><col style="width:60%"><col style="width:16%"><col style="width:12%"></colgroup>
    <thead><tr><th>排名</th><th>網域名稱</th><th>攔截次數</th><th>操作</th></tr></thead>
    <tbody><tr><td colspan=4 style="color:#8b949e">目前尚無攔截記錄</td></tr></tbody>
  </table>

  <h2>⚪ 網域白名單 (Allowlist - 信任允許)</h2>
  <div style="margin-bottom:8px">
    <input id=allowin placeholder="輸入信任網域，例如 tw.manhuagui.com" size=34>
    <button onclick="addAllowDom()" style="background:#238636;color:#fff;border-color:#2ea043">加入白名單</button>
  </div>
  <table id=at><colgroup><col style="width:80%"><col style="width:20%"></colgroup><tbody></tbody></table>

  <h2>🚫 自訂黑名單 (Custom Blocklist - 強制封鎖)</h2>
  <div style="margin-bottom:8px"><input id=dom placeholder="輸入廣告網域，例如 ads.example.com" size=34><button onclick=addDom()>加入黑名單</button></div>
  <table id=cl><colgroup><col style="width:80%"><col style="width:20%"></colgroup><tbody></tbody></table>

  <h2>💻 連線設備列表 (Connected Clients)</h2>
  <table id=ct>
    <colgroup><col style="width:26%"><col style="width:26%"><col style="width:18%"><col style="width:18%"><col style="width:12%"></colgroup>
    <thead><tr><th>設備 IP</th><th>MAC 位址</th><th>已攔截廣告</th><th>已允許查詢</th><th>管理</th></tr></thead>
    <tbody></tbody>
  </table>
</div>

<!-- ==================== 2. 系統設定專區分頁 ==================== -->
<div id="view-settings" class="tab-content">
  <h2>🌐 1. 上游 DNS 伺服器設定</h2>
  <div class=panel>
    <div style="display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px;margin-bottom:12px">
      <div><b>目前生效中:</b> <span id=activeup class=tag style="color:#58a6ff">讀取中...</span></div>
      <div>
        快速預設:
        <select id=uppreset onchange="applyUpstreamPreset()">
          <option value="quad9">Quad9 (9.9.9.9) - 惡意網站攔截保護</option>
          <option value="cloudflare">Cloudflare (1.1.1.1) - 極速隱私保護</option>
          <option value="google">Google DNS (8.8.8.8) - 穩定高可用</option>
          <option value="adguard">AdGuard DNS (94.140.14.14) - 上游過濾</option>
          <option value="custom">自訂 IP 與連接埠</option>
        </select>
      </div>
    </div>
    <div class=tool-row>
      <input id=upip placeholder="IP 位址 (例如 9.9.9.9)" size=18>
      <input id=upport placeholder="埠號 (預設 53)" size=8 value="53">
      <input id=upname placeholder="自訂名稱 (例如 Quad9)" size=14>
      <button onclick="saveUpstreamDns()" style="background:#238636;color:#fff;border-color:#2ea043">儲存上游 DNS</button>
    </div>
    <div id=upmsg class=section-desc>修改後即時動態生效，ESP32 無需重新開機。</div>
  </div>

  <h2>🌤️ 2. 天氣預報與地區設定</h2>
  <div class=panel>
    <div style="display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px;margin-bottom:12px">
      <div><b>目前地區:</b> <span id=curcity class=tag style="color:#58a6ff">台北 (Taipei)</span></div>
      <div>
        台灣主要城市:
        <select id=citypreset onchange="applyCityPreset()">
          <option value="taipei">台北市 (25.033, 121.565)</option>
          <option value="newtaipei">新北市 (25.012, 121.465)</option>
          <option value="taoyuan">桃園市 (24.993, 121.301)</option>
          <option value="hsinchu">新竹市 (24.803, 120.968)</option>
          <option value="taichung">台中市 (24.147, 120.673)</option>
          <option value="tainan">台南市 (22.999, 120.227)</option>
          <option value="kaohsiung">高雄市 (22.627, 120.301)</option>
          <option value="yilan">宜蘭縣 (24.757, 121.753)</option>
          <option value="hualien">花蓮縣 (23.977, 121.604)</option>
          <option value="taitung">台東縣 (22.756, 121.144)</option>
          <option value="penghu">澎湖縣 (23.565, 119.579)</option>
          <option value="custom">自訂經緯度</option>
        </select>
      </div>
    </div>
    <div class=tool-row>
      <input id=cfgcity placeholder="地區名稱 (例如 台北市)" size=18>
      <input id=cfglat placeholder="緯度 Latitude (例如 25.033)" size=12>
      <input id=cfglon placeholder="經度 Longitude (例如 121.565)" size=12>
      <button onclick="saveWeatherConfig()" style="background:#238636;color:#fff;border-color:#2ea043">儲存地區氣象設定</button>
    </div>
    <div id=wthrmsg class=section-desc>儲存後系統將自動連線 Open-Meteo 重新抓取並即時更新 AMOLED 螢幕時鐘天氣頁面。</div>
  </div>

  <h2>🖥️ 3. AMOLED 螢幕顯示與休眠管理</h2>
  <div class=panel>
    <div style="font-weight:600;margin-bottom:8px">螢幕休眠與電源管理 (AMOLED Sleep & Wake)</div>
    <div class="tool-row" style="margin-bottom:12px;gap:10px;align-items:center;flex-wrap:wrap">
      <span>自動休眠時間：</span>
      <select id=sleeptout style="background:#161b22;color:#c9d1d9;border:1px solid #30363d;padding:6px 10px;border-radius:6px">
        <option value="0">永不休眠 (螢幕常亮)</option>
        <option value="30">30 秒閒置後休眠</option>
        <option value="60">1 分鐘閒置後休眠</option>
        <option value="300" selected>5 分鐘閒置後休眠</option>
        <option value="600">10 分鐘閒置後休眠</option>
      </select>
      <button onclick="saveSleepTimeout()" style="background:#238636;color:#fff;border-color:#2ea043">儲存休眠排程</button>
      <button onclick="sleepNow()" style="background:#30363d;color:#c9d1d9">🌙 立即休眠</button>
      <button onclick="wakeNow()" style="background:#1f6feb;color:#fff;border-color:#388bfd">☀️ 立即喚醒</button>
      <span id=screenstate class=tag style="padding:4px 10px;border-radius:12px;font-size:12px;border:1px solid #238636;color:#3fb950;background:#142a1e">☀️ 螢幕顯示中</span>
    </div>
    <div class=section-desc style="margin-bottom:16px">
      AMOLED 深度休眠關閉像素發光與驅動晶片，有效防止面板烙印並降低功耗與發熱。點擊實體按鍵、觸控螢幕或網頁隨時可立即喚醒。
    </div>

    <div style="font-weight:600;margin-bottom:8px">螢幕畫面遠端切換</div>
    <div class="tool-row btn-grp" style="margin-bottom:14px">
      <button onclick="setScreenPage(0)">第 0 頁: 防護總覽</button>
      <button onclick="setScreenPage(1)">第 1 頁: 流量圖表</button>
      <button onclick="setScreenPage(2)">第 2 頁: 時鐘與天氣</button>
      <button onclick="setScreenPage(3)">第 3 頁: 攔截排行</button>
      <button onclick="setScreenPage(4)">第 4 頁: 硬體健康</button>
      <button onclick="setScreenPage(5)">第 5 頁: 掃碼連線</button>
      <button onclick="setScreenPage(6)" style="border-color:#00f0ff;color:#00f0ff;font-weight:600">第 6 頁: ✧ WuW ✧ 賽博哨兵</button>
    </div>

    <div style="font-weight:600;margin-bottom:8px">螢幕背光亮度與開機動畫</div>
    <div class="tool-row btn-grp">
      <button onclick="setBrightness(255)">☀️ 100% 全亮</button>
      <button onclick="setBrightness(170)">⛅ 66% 常用</button>
      <button onclick="setBrightness(75)">🌙 30% 微光</button>
      <button onclick="setBrightness(15)">🌑 5% 夜間低光</button>
      <button onclick="flipScreen()" style="margin-left:8px">🔄 翻轉螢幕 180°</button>
      <button onclick="replayBootAnim()" style="background:#8957e5;color:#fff;border-color:#a371f7;font-weight:600;margin-left:8px">🎬 重播 AMOLED 開機動畫</button>
    </div>
  </div>

  <h2>🔄 4. 廣告規則資料庫維護</h2>
  <div class=panel>
    <div style="font-weight:600;margin-bottom:6px">遠端定期自動更新 (Remote Auto-Update)</div>
    <div class=tool-row>
      <input id=uurl placeholder="https://host/blocklist.bin" style="flex:1;min-width:240px">
      每 <input id=uiv size=3 value=24> 小時
      <button onclick=saveUpd()>儲存排程</button>
      <button onclick=fetchNow() style="background:#1f6feb;color:#fff;border-color:#388bfd">立即下載更新</button>
    </div>
    <div class=section-desc>上次更新狀態：<span id=ustat>&mdash;</span></div>

    <div style="font-weight:600;margin-top:16px;margin-bottom:6px">手動上傳規則資料庫 (Blocklist Binary)</div>
    <form id=upf style="margin-bottom:6px">
      <input type=file id=blf accept=.bin>
      <button>上傳規則檔</button> <span id=upmsg style="color:#8b949e"></span>
    </form>
    <div class=section-desc>使用 <code>tools/build_blocklist.py</code> 建立 <code>blocklist.bin</code> 後由此直接上傳更新。</div>
  </div>

  <h2>🔑 5. 管理員安全設定 (Admin Credentials)</h2>
  <div class=panel>
    <div style="display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:8px">
      <input id=newuser placeholder="使用者名稱 (預設: admin)" size=18>
      <input type=password id=curpass placeholder="目前密碼" size=18>
      <input type=password id=newpass placeholder="新密碼 (至少 4 字元)" size=18>
      <button onclick="changePassword()" style="background:#238636;color:#fff;border-color:#2ea043">儲存新密碼</button>
    </div>
    <div id=pwdmsg style="font-size:13px;min-height:18px;color:#8b949e"></div>
    <div class=section-desc>帳號密碼保存於 ESP32 Flash 內部記憶體，用於控制台操作與無線 OTA 韌體更新。</div>
  </div>

  <h2>⚡ 6. 系統維護與操作</h2>
  <div class=panel>
    <div class="tool-row btn-grp">
      <button onclick="flushDnsCache()">🧹 清空內部 DNS 快取</button>
      <button onclick="rebootDevice()" style="background:#852222;color:#ffc4c4;border-color:#b53232">🔄 重新啟動設備 (Reboot)</button>
      <button onclick="if(confirm('確定要清除已儲存的 WiFi 設定並重新啟動進入配網模式嗎？'))forgetWifi()" style="background:#852222;color:#ffc4c4;border-color:#b53232">📡 清除 WiFi 設定並重啟</button>
    </div>

    <div style="font-weight:600;margin-top:16px;margin-bottom:6px">韌體無線升級 (Firmware OTA)</div>
    <form id=fwf style="margin-bottom:6px">
      <input type=file id=fwb accept=.bin>
      <button>上傳並刷寫韌體</button> <span id=fwmsg style="color:#8b949e"></span>
    </form>
    <div class=section-desc>上傳編譯後的 <code>.pio/build/t-display-s3-amoled/firmware.bin</code>，設備校驗完成後將自動重新開機。</div>
  </div>

  <h2>❄️ 7. 效能與節能降溫管理 (Thermal & Power Management)</h2>
  <div class=panel>
    <div style="display:flex;align-items:center;gap:12px;margin-bottom:12px;flex-wrap:wrap">
      <span style="font-weight:600">晶片核心溫度：</span>
      <span id=thermaltag class=tag style="padding:6px 14px;border-radius:14px;font-size:13px;font-weight:600">-- °C</span>
      <span style="color:#8b949e;font-size:13px">(正常運行溫度約 38~52°C)</span>
    </div>

    <div class="tool-row" style="margin-bottom:12px;gap:10px;align-items:center;flex-wrap:wrap">
      <span>CPU 運行頻率：</span>
      <select id=cpufreqsel style="background:#161b22;color:#c9d1d9;border:1px solid #30363d;padding:6px 10px;border-radius:6px">
        <option value="80">80 MHz (極致低溫 — 發熱最低，推薦)</option>
        <option value="160" selected>160 MHz (平衡模式 — 效能充沛且清涼，預設)</option>
        <option value="240">240 MHz (全速 240MHz — 高效能但發熱較大)</option>
      </select>

      <label style="display:flex;align-items:center;gap:6px;cursor:pointer;margin-left:8px">
        <input type=checkbox id=wifisleepchk checked>
        <span>Wi-Fi 數據機省電 (Modem Sleep)</span>
      </label>

      <button onclick="savePowerConfig()" style="background:#238636;color:#fff;border-color:#2ea043;margin-left:8px">儲存降溫設定</button>
    </div>

    <div class=section-desc>
      💡 <b>降溫指南</b>：ESP32-S3 雙核心預設全速 240MHz 運行且持續維持高頻 Wi-Fi 射頻時發熱量最大。切換至 <b>160 MHz 或 80 MHz</b> 並開啟 <b>Wi-Fi Modem Sleep</b>，可大幅降低功耗並使設備降溫 10~15°C，DNS 解析與畫面渲染完全零卡頓。
    </div>
  </div>

  <h2>🏠 8. 區網自訂網域解析 (Local DNS Records)</h2>
  <div class=panel>
    <div style="font-weight:600;margin-bottom:8px">自訂區網名稱與 IP 對應 (Local Domain Mapping)</div>
    <div class=tool-row style="margin-bottom:12px;gap:8px;flex-wrap:wrap">
      <input id=locdom placeholder="自訂網域 (例如 nas.lan 或 router.home)" style="flex:1;min-width:200px">
      <input id=locip placeholder="目標 IPv4 (例如 192.168.50.100)" size=18>
      <button onclick="addLocalDnsRecord()" style="background:#238636;color:#fff;border-color:#2ea043">+ 新增區網解析</button>
    </div>
    <div class=section-desc style="margin-bottom:12px">
      設定自訂網域後，內部區網設備查詢該網域將直接在 0.2ms 內由本機快速解析為指定 IP，無須繞經外網 DNS。支援各類 NAS、智慧家電、Proxmox/ESXi 伺服器或印表機。
    </div>
    <table id=localtb>
      <colgroup><col style="width:50%"><col style="width:35%"><col style="width:15%"></colgroup>
      <thead><tr><th>自訂網域名稱</th><th>目標 IP 位址</th><th>操作</th></tr></thead>
      <tbody><tr><td colspan=3 style="color:#8b949e">目前尚未建立自訂區網解析記錄</td></tr></tbody>
    </table>
  </div>
</div>

</div>

<script>
function fmt(n){return (n||0).toLocaleString()}
function esc(s){return String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
const CSRF_HDRS={'X-Requested-With':'c3-adblock'};

function switchTab(id){
  document.querySelectorAll('.nav-tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tab-content').forEach(c=>c.classList.remove('active'));
  let btn = document.getElementById('tab-'+id);
  let view = document.getElementById('view-'+id);
  if(btn) btn.classList.add('active');
  if(view) view.classList.add('active');
  window.location.hash = id;
}
if(window.location.hash === '#settings') switchTab('settings');

function togglePause(){fetch(blockstate.dataset.on=='1'?'/pause?s='+pausedur.value:'/resume',{headers:CSRF_HDRS}).then(load);}
function flipScreen(){fetch('/flip',{headers:CSRF_HDRS});}
function replayBootAnim(){fetch('/replayboot',{headers:CSRF_HDRS}).then(()=>{alert('✨ 已觸發 T-Display-S3 AMOLED 賽博開機動畫重播！');});}
function setScreenPage(p){fetch('/setpage?p='+p,{headers:CSRF_HDRS});}
function setBrightness(b){fetch('/setbrightness?b='+b,{headers:CSRF_HDRS});}
function sleepNow(){fetch('/sleep',{headers:CSRF_HDRS}).then(load);}
function wakeNow(){fetch('/wakeup',{headers:CSRF_HDRS}).then(load);}
async function saveSleepTimeout(){
  let s=sleeptout.value;
  try{
    let r=await fetch('/setsleep?s='+encodeURIComponent(s),{headers:CSRF_HDRS});
    if(r.ok){
      alert('已更新螢幕自動休眠設定！');
      load();
    }
  }catch(_){}
}
async function savePowerConfig(){
  let c = cpufreqsel.value;
  let ws = wifisleepchk.checked ? 1 : 0;
  try {
    let r = await fetch('/setpower?cpu='+c+'&wsleep='+ws, {headers: CSRF_HDRS});
    if(r.ok){
      alert('已成功更新降溫與節能設定！');
      load();
    }
  }catch(_){}
}
function flushDnsCache(){fetch('/flushcache',{headers:CSRF_HDRS}).then(()=>{testres.innerHTML='<span style="color:#3fb950">✓ DNS 快取已成功清空！</span>';setTimeout(load,300);});}

async function rebootDevice(){
  if(!confirm('確定要重新啟動 ESP32 設備嗎？')) return;
  try {
    await fetch('/reboot', {headers: CSRF_HDRS});
    alert('設備正在重新開機，請於約 10-15 秒後重新整理網頁。');
    setTimeout(()=>location.reload(), 12000);
  } catch(e){ alert('重新開機請求已發送'); }
}

async function runDnsTest(){
  let d = testdom.value.trim();
  if(!d){testres.textContent='請輸入網域名稱';return;}
  testres.style.color='#8b949e';testres.textContent='正在測試網域解析與比對...';
  try {
    let r = await fetch('/dnstest?d='+encodeURIComponent(d));
    let j = await r.json();
    if(j.blocked){
      testres.innerHTML = `<span style="color:#f85149;font-weight:600">⛔ 已封鎖攔截</span> (${esc(j.reason)}) &mdash; DNS 查詢回應 0.0.0.0`;
    } else {
      testres.innerHTML = `<span style="color:#3fb950;font-weight:600">✅ 允許通過</span> (${esc(j.reason)}) &mdash; 已轉發至上游伺服器 (${esc(j.upstream)})`;
    }
  } catch(e){
    testres.innerHTML = '<span style="color:#f85149">測試請求失敗，請檢查網路連線</span>';
  }
}

const UPSTREAM_PRESETS = {
  quad9: {ip:'9.9.9.9', port:53, name:'Quad9'},
  cloudflare: {ip:'1.1.1.1', port:53, name:'Cloudflare'},
  google: {ip:'8.8.8.8', port:53, name:'Google DNS'},
  adguard: {ip:'94.140.14.14', port:53, name:'AdGuard DNS'}
};
function applyUpstreamPreset(){
  let v = uppreset.value;
  if(UPSTREAM_PRESETS[v]){
    upip.value = UPSTREAM_PRESETS[v].ip;
    upport.value = UPSTREAM_PRESETS[v].port;
    upname.value = UPSTREAM_PRESETS[v].name;
  }
}
async function saveUpstreamDns(){
  let ip = upip.value.trim(), port = upport.value.trim()||53, name = upname.value.trim()||'Custom';
  if(!ip){alert('請輸入有效的 IP 位址');return;}
  upmsg.textContent = '正在更新上游 DNS...';
  try {
    let r = await fetch('/setupstream?ip='+encodeURIComponent(ip)+'&port='+encodeURIComponent(port)+'&name='+encodeURIComponent(name),{headers:CSRF_HDRS});
    if(r.ok){
      upmsg.style.color='#3fb950';upmsg.textContent='✓ 上游 DNS 更新成功！已立即生效。';
      setTimeout(load, 400);
    } else {
      upmsg.style.color='#f85149';upmsg.textContent='✗ 更新失敗: '+await r.text();
    }
  } catch(e){
    upmsg.style.color='#f85149';upmsg.textContent='✗ 網路錯誤';
  }
}

const CITY_PRESETS = {
  taipei: {city:'台北市', lat:'25.033', lon:'121.565'},
  newtaipei: {city:'新北市', lat:'25.012', lon:'121.465'},
  taoyuan: {city:'桃園市', lat:'24.993', lon:'121.301'},
  hsinchu: {city:'新竹市', lat:'24.803', lon:'120.968'},
  taichung: {city:'台中市', lat:'24.147', lon:'120.673'},
  tainan: {city:'台南市', lat:'22.999', lon:'120.227'},
  kaohsiung: {city:'高雄市', lat:'22.627', lon:'120.301'},
  yilan: {city:'宜蘭縣', lat:'24.757', lon:'121.753'},
  hualien: {city:'花蓮縣', lat:'23.977', lon:'121.604'},
  taitung: {city:'台東縣', lat:'22.756', lon:'121.144'},
  penghu: {city:'澎湖縣', lat:'23.565', lon:'119.579'}
};
function applyCityPreset(){
  let v = citypreset.value;
  if(CITY_PRESETS[v]){
    cfgcity.value = CITY_PRESETS[v].city;
    cfglat.value = CITY_PRESETS[v].lat;
    cfglon.value = CITY_PRESETS[v].lon;
  }
}
async function saveWeatherConfig(){
  let c = cfgcity.value.trim()||'台北市', la = cfglat.value.trim()||'25.033', lo = cfglon.value.trim()||'121.565';
  wthrmsg.textContent = '正在儲存地區氣象設定並更新天氣...';
  try {
    let r = await fetch('/setweather?city='+encodeURIComponent(c)+'&lat='+encodeURIComponent(la)+'&lon='+encodeURIComponent(lo), {headers: CSRF_HDRS});
    if(r.ok){
      wthrmsg.style.color='#3fb950';wthrmsg.textContent='✓ 地區氣象設定成功儲存！已即時連線刷新氣象資訊。';
      setTimeout(load, 500);
    } else {
      wthrmsg.style.color='#f85149';wthrmsg.textContent='✗ 儲存失敗: '+await r.text();
    }
  } catch(e){
    wthrmsg.style.color='#f85149';wthrmsg.textContent='✗ 請求失敗';
  }
}

let inFlight=false,lastCustom='',lastAllow='',lastTop='',lastLocalDns='';
async function load(){
  if(inFlight)return;inFlight=true;
  try{
    let res=await fetch('/stats.json');if(!res.ok)throw 0;let s=await res.json();
    if(host.textContent!=='@ '+s.ip)host.textContent='@ '+s.ip;
    credwarn.style.display=s.defcreds?'block':'none';
    if(s.webuser&&document.activeElement!=newuser&&!newuser.value)newuser.value=s.webuser;
    let on=s.blocking!==false;blockstate.dataset.on=on?'1':'0';
    let dot=on?'🛡️':'⏸️';if(blockdot.textContent!==dot)blockdot.textContent=dot;
    blockbar.style.borderColor=on?'#30363d':'#f0883e';
    let stMsg=on?'廣告攔截防護中':(s.resumeIn>0?'攔截已暫停 — 將於 '+s.resumeIn+' 秒後恢復':'攔截已暫停');
    if(blockstate.textContent!==stMsg)blockstate.textContent=stMsg;
    pausebtn.textContent=on?'暫停攔截':'恢復攔截';pausedur.style.display=on?'':'none';

    // Metrics cards
    v_blocked.textContent=fmt(s.blocked);
    v_allowed.textContent=fmt(s.allowed);
    let tot = (s.blocked||0) + (s.allowed||0);
    v_rate.textContent = tot>0 ? ((s.blocked*100/tot).toFixed(1)+'%') : '0.0%';
    v_domains.textContent=fmt(s.domains)+' 條';
    v_clients.textContent=s.clients.length+' 台';
    v_wifi.textContent=s.rssi+' dBm';
    v_temp.textContent=s.temp+' °C';
    v_heap.textContent=Math.round(s.heap/1024)+' KB';
    v_psram.textContent=Math.round((s.psram||0)/1024)+' KB';
    v_uptime.textContent=s.uptime;
    if(window.v_qps) v_qps.innerHTML = (s.qps !== undefined ? s.qps.toFixed(1) : '0.0') + ' <span style="font-size:12px;color:#8b949e">q/s</span>';
    if(window.v_bps) {
      let kb = (s.bps !== undefined ? (s.bps / 1024.0).toFixed(1) : '0.0');
      v_bps.innerHTML = kb + ' <span style="font-size:12px;color:#8b949e">KB/s</span>';
    }
    if(window.v_peak) v_peak.innerHTML = (s.peak_qps !== undefined ? s.peak_qps.toFixed(1) : '0.0') + ' <span style="font-size:12px;color:#8b949e">q/s</span>';

    // Ping Sentinel WAN Latency & Alert
    if(s.netOnline !== undefined && window.v_ping){
      if(!s.netOnline){
        v_ping.innerHTML = '<span style="color:#f85149">⚠️ 斷網</span>';
        if(window.wanalert) {
          wanalert.style.display = 'flex';
          if(s.offlineSec && window.offline_timer) offline_timer.textContent = `已斷線 ${s.offlineSec} 秒`;
        }
      } else {
        let rtt = s.pingRtt || 0;
        let color = rtt < 35 ? '#3fb950' : (rtt < 80 ? '#58a6ff' : '#d29922');
        v_ping.innerHTML = `<span style="color:${color}">${rtt} ms</span>`;
        if(window.wanalert) wanalert.style.display = 'none';
      }
    }

    // Weather Header & Settings
    if(s.weather && s.weather.valid){
      let cName = s.weather.city || '台北';
      wthr.innerHTML = `🌤️ ${esc(cName)}: <b>${s.weather.temp.toFixed(1)}°C</b> (${esc(s.weather.cond)}) &bull; 濕度: ${s.weather.hum}%`;
      if(curcity) curcity.textContent = cName;
      if(document.activeElement!=cfgcity&&!cfgcity.value) cfgcity.value = s.weather.city||'';
      if(document.activeElement!=cfglat&&!cfglat.value) cfglat.value = s.weather.lat||'';
      if(document.activeElement!=cfglon&&!cfglon.value) cfglon.value = s.weather.lon||'';
    }

    // Upstream DNS Active Display
    if(s.upstream){
      activeup.textContent = `${s.upstream.name} (${s.upstream.ip}:${s.upstream.port})`;
      if(document.activeElement!=upip&&!upip.value)upip.value=s.upstream.ip;
      if(document.activeElement!=upport&&!upport.value)upport.value=s.upstream.port;
      if(document.activeElement!=upname&&!upname.value)upname.value=s.upstream.name;
    }

    // Screen Sleep & Timeout State
    if(s.screenAsleep !== undefined && screenstate){
      let asleep = (s.screenAsleep === true);
      screenstate.textContent = asleep ? '🌙 螢幕休眠中' : '☀️ 螢幕顯示中';
      screenstate.style.color = asleep ? '#a371f7' : '#3fb950';
      screenstate.style.borderColor = asleep ? '#8957e5' : '#238636';
      screenstate.style.background = asleep ? '#261b3d' : '#142a1e';
    }
    if(s.sleepTimeout !== undefined && sleeptout && document.activeElement != sleeptout){
      sleeptout.value = s.sleepTimeout;
    }

    // Thermal & Power Management State
    if(s.temp !== undefined && window.thermaltag){
      let t = parseFloat(s.temp);
      let color = '#3fb950', bg = '#142a1e', border = '#238636', desc = '清涼正常';
      if(t >= 62){ color = '#f85149'; bg = '#3d1618'; border = '#da3633'; desc = '過熱警戒'; }
      else if(t >= 53){ color = '#d29922'; bg = '#2c220f'; border = '#9e6a03'; desc = '偏熱微溫'; }
      thermaltag.textContent = `🌡️ ${t.toFixed(1)} °C (${desc})`;
      thermaltag.style.color = color;
      thermaltag.style.background = bg;
      thermaltag.style.borderColor = border;
    }
    if(s.cpufreq !== undefined && window.cpufreqsel && document.activeElement != cpufreqsel){
      cpufreqsel.value = s.cpufreq;
    }
    if(s.wifisleep !== undefined && window.wifisleepchk && document.activeElement != wifisleepchk){
      wifisleepchk.checked = (s.wifisleep === true);
    }

    // Clients Table (in-place update)
    let tb=ct.tBodies[0],cur=new Set();
    s.clients.forEach(c=>{
      cur.add(c.ip);let row=tb.querySelector(`tr[data-ip="${c.ip}"]`);
      if(!row){row=document.createElement('tr');row.dataset.ip=c.ip;
      row.innerHTML=`<td class=cip></td><td class=cmac></td><td class="b cblk"></td><td class="a calw"></td><td><button class=ban data-ip="${c.ip}"></button></td>`;
      tb.appendChild(row);}
      let ipH=esc(c.ip)+(c.banned?' <span class=tag style=color:#f85149>已封鎖</span>':'');
      let tdIp=row.querySelector('.cip');if(tdIp.innerHTML!==ipH)tdIp.innerHTML=ipH;
      let tdMac=row.querySelector('.cmac');if(tdMac.textContent!==c.mac)tdMac.textContent=c.mac;
      let tdB=row.querySelector('.cblk'),bS=fmt(c.blocked);if(tdB.textContent!==bS)tdB.textContent=bS;
      let tdA=row.querySelector('.calw'),aS=fmt(c.allowed);if(tdA.textContent!==aS)tdA.textContent=aS;
      let btn=row.querySelector('.ban'),wT=c.banned?'解除封鎖':'封鎖設備';if(btn.textContent!==wT)btn.textContent=wT;
    });
    Array.from(tb.children).forEach(r=>{if(r.dataset.ip&&!cur.has(r.dataset.ip))r.remove()});

    // Query Logs Table
    if(s.logs && s.logs.length){
      lt.tBodies[0].innerHTML = s.logs.map(l=>{
        let st = l.b ? '<span class=badge-blk>已攔截</span>' : (l.loc ? '<span class=badge-local>區網解析</span>' : '<span class=badge-alw>已轉發</span>');
        let act = l.b ? `<button class="alwbtn" data-d="${esc(l.d)}" style="padding:2px 8px;font-size:11px">+白名單</button>`
                      : `<button class="blkbtn" data-d="${esc(l.d)}" style="padding:2px 8px;font-size:11px">+黑名單</button>`;
        return `<tr><td style="color:#8b949e">${l.t ? (l.t%86400<10?'0':'')+Math.floor((l.t%3600)/60)+'分 '+(l.t%60)+'秒' : '剛剛'}</td><td>${esc(l.ip)}</td><td title="${esc(l.d)}">${esc(l.d)}</td><td>${st}</td><td>${act}</td></tr>`;
      }).join('');
    }

    // Top Blocked Table
    let topStr = JSON.stringify(s.topBlocked||[]);
    if(topStr !== lastTop){
      lastTop = topStr;
      if(s.topBlocked && s.topBlocked.length){
        tbt.tBodies[0].innerHTML = s.topBlocked.map((t,idx)=>`<tr><td><b>#${idx+1}</b></td><td title="${esc(t.domain)}">${esc(t.domain)}</td><td class=b>${fmt(t.count)} 次</td><td><button class=alwbtn data-d="${esc(t.domain)}" style="padding:2px 8px;font-size:11px">+白名單</button></td></tr>`).join('');
      } else {
        tbt.tBodies[0].innerHTML = '<tr><td colspan=4 style="color:#8b949e">目前尚無攔截記錄</td></tr>';
      }
    }

    // Allowlist Table
    let aStr = (s.allow||[]).join('|');
    if(aStr !== lastAllow){
      lastAllow = aStr;
      at.tBodies[0].innerHTML = (s.allow&&s.allow.length)?s.allow.map(d=>`<tr><td>${esc(d)}</td><td style="text-align:right"><button class=rmalwbtn data-d="${esc(d)}">移除</button></td></tr>`).join(''):'<tr><td style="color:#8b949e">目前尚未加入任何白名單網域</td></tr>';
    }

    // Custom Block Table
    let cStr = (s.custom||[]).join('|');
    if(cStr !== lastCustom){
      lastCustom = cStr;
      cl.tBodies[0].innerHTML=(s.custom&&s.custom.length)?s.custom.map(d=>`<tr><td>${esc(d)}</td><td style="text-align:right"><button class=rmbtn data-d="${esc(d)}">移除</button></td></tr>`).join(''):'<tr><td style="color:#8b949e">目前尚未加入任何自訂黑名單</td></tr>';
    }

    // Local DNS Records Table
    let locStr = JSON.stringify(s.localdns||[]);
    if(locStr !== lastLocalDns && window.localtb){
      lastLocalDns = locStr;
      localtb.tBodies[0].innerHTML = (s.localdns && s.localdns.length) ? s.localdns.map(item => `<tr><td><b>${esc(item.d)}</b></td><td><code>${esc(item.ip)}</code></td><td style="text-align:right"><button class="rmlocalbtn" data-d="${esc(item.d)}" style="padding:2px 8px;font-size:11px">移除</button></td></tr>`).join('') : '<tr><td colspan=3 style="color:#8b949e">目前尚未建立自訂區網解析記錄</td></tr>';
    }

    if(document.activeElement!=uurl)uurl.value=s.upurl||'';
    if(document.activeElement!=uiv)uiv.value=s.upiv||24;
    if(ustat.textContent!==(s.upstat||'—'))ustat.textContent=s.upstat||'—';
  }catch(_){}finally{inFlight=false;}
}

async function changePassword(){
  let u=newuser.value.trim(), c=curpass.value, p=newpass.value;
  if(!p || p.length < 4){ alert('新密碼長度至少需 4 個字元 (Password must be at least 4 characters)'); return; }
  pwdmsg.style.color='#8b949e';pwdmsg.textContent='正在更新密碼...';
  try {
    let r = await fetch('/setpassword?newpass=' + encodeURIComponent(p) + '&curpass=' + encodeURIComponent(c) + (u ? ('&newuser=' + encodeURIComponent(u)) : ''), {
      method: 'POST',
      headers: CSRF_HDRS
    });
    if(r.ok) {
      pwdmsg.style.color='#3fb950';pwdmsg.textContent='✓ 密碼修改成功！';
      curpass.value='';newpass.value='';
      setTimeout(()=>{ alert('密碼修改成功！日後請使用新密碼進行管理與更新。'); }, 50);
    } else {
      pwdmsg.style.color='#f85149';
      pwdmsg.textContent='✗ 修改失敗: ' + await r.text();
    }
  } catch(e) {
    pwdmsg.style.color='#f85149';pwdmsg.textContent='✗ 請求失敗，請檢查網路連線';
  }
}

function addDom(){let d=dom.value.trim();if(d){fetch('/addblock?d='+encodeURIComponent(d),{headers:CSRF_HDRS}).then(()=>{dom.value='';load()})}}
function addAllowDom(){let d=allowin.value.trim();if(d){fetch('/addallow?d='+encodeURIComponent(d),{headers:CSRF_HDRS}).then(()=>{allowin.value='';load()})}}

async function addLocalDnsRecord(){
  let d = locdom.value.trim(), ip = locip.value.trim();
  if(!d || !ip){ alert('請輸入自訂網域名稱與目標 IPv4 位址'); return; }
  try {
    let r = await fetch('/addlocaldns?d=' + encodeURIComponent(d) + '&ip=' + encodeURIComponent(ip), {headers: CSRF_HDRS});
    if(r.ok){ locdom.value = ''; locip.value = ''; load(); }
    else { alert('新增失敗: ' + await r.text()); }
  } catch(e){ alert('請求失敗，請檢查網路連線'); }
}

ct.addEventListener('click',e=>{if(e.target.classList.contains('ban'))fetch('/ban?ip='+e.target.dataset.ip,{headers:CSRF_HDRS}).then(load)});
cl.addEventListener('click',e=>{if(e.target.classList.contains('rmbtn'))fetch('/unblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});
at.addEventListener('click',e=>{if(e.target.classList.contains('rmalwbtn'))fetch('/unallow?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});

document.addEventListener('click',e=>{
  if(e.target.classList.contains('alwbtn')){
    fetch('/addallow?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load);
  } else if(e.target.classList.contains('blkbtn')){
    fetch('/addblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load);
  } else if(e.target.classList.contains('rmlocalbtn')){
    let d = e.target.dataset.d;
    if(confirm(`確定要移除區網解析記錄 [${d}] 嗎？`)){
      fetch('/dellocaldns?d='+encodeURIComponent(d),{headers:CSRF_HDRS}).then(load);
    }
  }
});

function saveUpd(){fetch('/setupdate?u='+encodeURIComponent(uurl.value.trim())+'&h='+(parseInt(uiv.value)||24),{headers:CSRF_HDRS}).then(load)}
function fetchNow(){ustat.textContent='下載更新中...';fetch('/fetchnow',{headers:CSRF_HDRS}).then(r=>r.text()).then(t=>{ustat.textContent=t;load()})}
function forgetWifi(){fetch('/forgetwifi',{headers:CSRF_HDRS}).then(r=>r.text()).then(t=>alert(t))}

fwf.onsubmit=async e=>{e.preventDefault();let f=fwb.files[0];if(!f)return;fwmsg.textContent='正在刷寫韌體 '+(f.size/1048576).toFixed(2)+' MB...';
  let fd=new FormData();fd.append('f',f);
  try{let r=await fetch('/update',{method:'POST',headers:CSRF_HDRS,body:fd});fwmsg.textContent=r.ok?'✓ 更新成功！設備重新啟動中，請於約 15 秒後重新整理網頁':'✗ '+await r.text();}
  catch(_){fwmsg.textContent='✓ 更新成功！設備重新啟動中，請於約 15 秒後重新整理網頁';}};

upf.onsubmit=async e=>{e.preventDefault();let f=blf.files[0];if(!f)return;
  upmsg.textContent='正在上傳規則檔 '+(f.size/1048576).toFixed(2)+' MB...';
  let fd=new FormData();fd.append('f',f);
  try{let r=await fetch('/upload',{method:'POST',headers:CSRF_HDRS,body:fd});upmsg.textContent=r.ok?'✓ 上傳更新成功！':'✗ '+await r.text();}
  catch(_){upmsg.textContent='✗ 上傳失敗';}
  blf.value='';setTimeout(load,600);};

load();setInterval(load,3000);
</script></body></html>)HTML";
