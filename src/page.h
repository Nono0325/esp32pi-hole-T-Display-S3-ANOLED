#pragma once
// Dashboard HTML for the T-Display-S3 AMOLED AdBlocker web UI.
// Built for rock-solid stability with zero layout shift, in-place DOM updates,
// tabular numbers, fixed column layouts, and smooth real-time refreshes.

const char PAGE[] PROGMEM = R"HTML(<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>T-Display-S3 AMOLED AdBlock</title><style>
*{box-sizing:border-box}
html,body{font:14px system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;margin:0;background:#0d1117;color:#c9d1d9;overflow-y:scroll}
header{background:#161b22;padding:14px 18px;border-bottom:1px solid #30363d;display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px}
h1{margin:0;font-size:18px}h1 span{color:#3fb950}.wrap{padding:16px;max-width:1050px;margin:auto}
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
button{background:#21262d;color:#c9d1d9;border:1px solid #30363d;border-radius:5px;padding:5px 10px;cursor:pointer;font-size:13px}
button:hover{background:#30363d;border-color:#8b949e}.ban{color:#f85149}
input,select{background:#0d1117;border:1px solid #30363d;color:#c9d1d9;border-radius:5px;padding:6px}
input:focus,select:focus{border-color:#58a6ff;outline:none}
h2{font-size:14px;color:#8b949e;margin:20px 0 8px;display:flex;align-items:center;gap:6px}
#blockbar{display:flex;align-items:center;gap:12px;margin-bottom:14px;padding:12px 14px;background:#161b22;border:1px solid #30363d;border-radius:8px;min-height:54px;flex-wrap:wrap}
.panel{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:14px;margin-bottom:18px}
.btn-grp{display:flex;flex-wrap:wrap;gap:8px}
.tool-row{display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:10px}
</style></head><body>
<header>
  <h1>🛡️ T-Display-S3 AMOLED AdBlock <span id=host></span></h1>
  <div id=wthr style="font-size:13px;color:#8b949e">🌤️ Weather: Loading...</div>
</header>
<div class=wrap>
<div id=credwarn style="display:none;background:#3b1d1d;border:1px solid #f85149;color:#ffb3ae;border-radius:8px;padding:10px 14px;margin-bottom:14px;font-size:13px">
⚠️ <b>Default credentials in use.</b> Change your password in the ADMIN section below.
</div>

<div id=blockbar>
  <span id=blockdot style="font-size:20px;line-height:1">🛡️</span><b id=blockstate style="flex:1" data-on=1>Blocking active</b>
  <select id=pausedur><option value=30>30s</option><option value=300 selected>5 min</option><option value=1800>30 min</option><option value=0>until I re-enable</option></select>
  <button id=pausebtn onclick=togglePause()>Pause</button>
  <button id=flipbtn onclick=flipScreen() title="Flip Screen 180°">🔄 Flip 180°</button>
</div>

<div class=cards id=sys>
  <div class=card><div class="v b" id=v_blocked>0</div><div class=l>Blocked Ads</div></div>
  <div class=card><div class="v a" id=v_allowed>0</div><div class=l>Allowed Queries</div></div>
  <div class=card><div class=v id=v_rate>0.0%</div><div class=l>Block Rate</div></div>
  <div class=card><div class=v id=v_domains>0</div><div class=l>Blocklist Rules</div></div>
  <div class=card><div class=v id=v_clients>0</div><div class=l>Clients</div></div>
  <div class=card><div class=v id=v_wifi>--</div><div class=l>WiFi Signal</div></div>
  <div class=card><div class=v id=v_temp>--</div><div class=l>SoC Temp</div></div>
  <div class=card><div class=v id=v_heap>--</div><div class=l>Free RAM</div></div>
  <div class=card><div class=v id=v_psram>--</div><div class=l>Free PSRAM</div></div>
  <div class=card><div class=v id=v_uptime>--</div><div class=l>Uptime</div></div>
</div>

<h2>🛠️ NETWORK TOOLS & REMOTE CONTROLS (小工具專區)</h2>
<div class=panel>
  <div style="font-weight:600;margin-bottom:6px">1. DNS Resolution Diagnostic Tool (網域攔截測試)</div>
  <div class=tool-row>
    <input id=testdom placeholder="e.g. adservice.google.com" style="flex:1;min-width:220px">
    <button onclick="runDnsTest()" style="background:#1f6feb;color:#fff;border-color:#388bfd">Test Domain</button>
    <button onclick="flushDnsCache()" title="Clear internal memory cache">🧹 Flush DNS Cache</button>
  </div>
  <div id=testres style="font-size:13px;min-height:18px;margin-bottom:14px;color:#8b949e"></div>

  <div style="font-weight:600;margin-bottom:6px">2. Remote AMOLED Screen Switcher (螢幕畫面切換)</div>
  <div class="tool-row btn-grp" style="margin-bottom:12px">
    <button onclick="setScreenPage(0)">Page 0: Overview</button>
    <button onclick="setScreenPage(1)">Page 1: Histogram</button>
    <button onclick="setScreenPage(2)">Page 2: Clock & Weather</button>
    <button onclick="setScreenPage(3)">Page 3: Top Blocked</button>
    <button onclick="setScreenPage(4)">Page 4: Hardware Health</button>
    <button onclick="setScreenPage(5)">Page 5: QR Connect</button>
  </div>

  <div style="font-weight:600;margin-bottom:6px">3. AMOLED Brightness (亮度控制)</div>
  <div class="tool-row btn-grp">
    <button onclick="setBrightness(255)">☀️ 100%</button>
    <button onclick="setBrightness(170)">⛅ 66%</button>
    <button onclick="setBrightness(75)">🌙 30%</button>
    <button onclick="setBrightness(15)">🌑 5% (Low)</button>
  </div>
</div>

<h2>🌐 UPSTREAM DNS SERVER (動態上游伺服器)</h2>
<div class=panel>
  <div style="display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px;margin-bottom:10px">
    <div><b>Active DNS:</b> <span id=activeup class=tag style="color:#58a6ff">Loading...</span></div>
    <div>
      Preset:
      <select id=uppreset onchange="applyUpstreamPreset()">
        <option value="quad9">Quad9 (9.9.9.9) - Malware Block</option>
        <option value="cloudflare">Cloudflare (1.1.1.1) - Privacy / Fast</option>
        <option value="google">Google DNS (8.8.8.8) - High Reliability</option>
        <option value="adguard">AdGuard DNS (94.140.14.14) - Upstream Ad Filter</option>
        <option value="custom">Custom IP / Port</option>
      </select>
    </div>
  </div>
  <div class=tool-row>
    <input id=upip placeholder="IP (e.g. 9.9.9.9)" size=18>
    <input id=upport placeholder="Port (53)" size=6 value="53">
    <input id=upname placeholder="Name (e.g. Quad9)" size=14>
    <button onclick="saveUpstreamDns()" style="background:#238636;color:#fff;border-color:#2ea043">Save Upstream</button>
  </div>
  <div id=upmsg style="font-size:12px;color:#8b949e">Upstream queries are forwarded dynamically without restarting the ESP32.</div>
</div>

<h2>📊 REAL-TIME DNS QUERY LOG (即時查詢日誌)</h2>
<table id=lt>
  <colgroup><col style="width:12%"><col style="width:20%"><col style="width:46%"><col style="width:12%"><col style="width:10%"></colgroup>
  <thead><tr><th>Time</th><th>Client</th><th>Domain</th><th>Status</th><th>Action</th></tr></thead>
  <tbody><tr><td colspan=5 style="color:#8b949e">Waiting for queries...</td></tr></tbody>
</table>

<h2>🏆 TOP BLOCKED DOMAINS (熱門攔截排行)</h2>
<table id=tbt>
  <colgroup><col style="width:12%"><col style="width:60%"><col style="width:16%"><col style="width:12%"></colgroup>
  <thead><tr><th>Rank</th><th>Domain</th><th>Hits</th><th>Action</th></tr></thead>
  <tbody><tr><td colspan=4 style="color:#8b949e">No blocked queries recorded yet</td></tr></tbody>
</table>

<h2>⚪ ALLOWLIST / WHITELIST (網域白名單)</h2>
<div style="margin-bottom:8px">
  <input id=allowin placeholder="domain.com (bypasses all blocking)" size=32>
  <button onclick="addAllowDom()" style="background:#238636;color:#fff;border-color:#2ea043">Whitelist Domain</button>
</div>
<table id=at><colgroup><col style="width:80%"><col style="width:20%"></colgroup><tbody></tbody></table>

<h2>🚫 CUSTOM BLOCKED DOMAINS (自訂黑名單)</h2>
<div style="margin-bottom:8px"><input id=dom placeholder="ads.example.com" size=32><button onclick=addDom()>Block domain</button></div>
<table id=cl><colgroup><col style="width:80%"><col style="width:20%"></colgroup><tbody></tbody></table>

<h2>💻 CONNECTED CLIENTS (連線設備)</h2>
<table id=ct>
  <colgroup><col style="width:26%"><col style="width:26%"><col style="width:18%"><col style="width:18%"><col style="width:12%"></colgroup>
  <thead><tr><th>Client</th><th>MAC</th><th>Blocked</th><th>Allowed</th><th>Action</th></tr></thead>
  <tbody></tbody>
</table>

<h2>📦 BLOCKLIST &mdash; UPLOAD BINARY</h2>
<form id=upf style="margin-bottom:6px"><input type=file id=blf accept=.bin><button>Upload blocklist</button> <span id=upmsg style="color:#8b949e"></span></form>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">build <code>blocklist.bin</code> with <code>tools/build_blocklist.py</code>, then upload here &mdash; no USB</div>

<h2>🔄 BLOCKLIST &mdash; REMOTE AUTO-UPDATE</h2>
<div style="margin-bottom:6px"><input id=uurl placeholder="https://host/blocklist.bin" size=40> every <input id=uiv size=2 value=24>h
<button onclick=saveUpd()>Save</button> <button onclick=fetchNow()>Fetch now</button></div>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">device pulls a prebuilt <code>blocklist.bin</code> on a schedule (e.g. a GitHub release asset). last: <span id=ustat>&mdash;</span></div>

<h2>⚡ FIRMWARE &mdash; OTA UPDATE</h2>
<form id=fwf style="margin-bottom:6px"><input type=file id=fwb accept=.bin><button>Flash firmware</button> <span id=fwmsg style="color:#8b949e"></span></form>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">upload <code>.pio/build/t-display-s3-amoled/firmware.bin</code> &mdash; device verifies it and reboots</div>

<h2>🔑 ADMIN &amp; PASSWORD</h2>
<div class=panel>
  <div style="display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:8px">
    <input id=newuser placeholder="Username (default: admin)" size=18>
    <input type=password id=curpass placeholder="Current password" size=18>
    <input type=password id=newpass placeholder="New password (min 4)" size=18>
    <button onclick="changePassword()" style="background:#238636;color:#fff;border-color:#2ea043">Save Password</button>
  </div>
  <div id=pwdmsg style="font-size:13px;min-height:18px;color:#8b949e"></div>
  <div style="color:#8b949e;font-size:12px;margin-top:4px">Credentials are stored in ESP32 flash memory for dashboard control and OTA firmware updates.</div>
</div>

<h2>📡 NETWORK RESET</h2>
<div style="margin-bottom:24px"><button onclick="if(confirm('Forget saved WiFi and reboot into the setup portal?'))forgetWifi()">Forget WiFi</button></div>
</div>

<script>
function fmt(n){return (n||0).toLocaleString()}
function esc(s){return String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
const CSRF_HDRS={'X-Requested-With':'c3-adblock'};

function togglePause(){fetch(blockstate.dataset.on=='1'?'/pause?s='+pausedur.value:'/resume',{headers:CSRF_HDRS}).then(load);}
function flipScreen(){fetch('/flip',{headers:CSRF_HDRS});}
function setScreenPage(p){fetch('/setpage?p='+p,{headers:CSRF_HDRS});}
function setBrightness(b){fetch('/setbrightness?b='+b,{headers:CSRF_HDRS});}
function flushDnsCache(){fetch('/flushcache',{headers:CSRF_HDRS}).then(()=>{testres.innerHTML='<span style="color:#3fb950">✓ DNS Cache flushed successfully</span>';setTimeout(load,300);});}

async function runDnsTest(){
  let d = testdom.value.trim();
  if(!d){testres.textContent='Please enter a domain name';return;}
  testres.style.color='#8b949e';testres.textContent='Testing domain resolution...';
  try {
    let r = await fetch('/dnstest?d='+encodeURIComponent(d));
    let j = await r.json();
    if(j.blocked){
      testres.innerHTML = `<span style="color:#f85149;font-weight:600">⛔ BLOCKED</span> (${esc(j.reason)}) &mdash; Query returned 0.0.0.0`;
    } else {
      testres.innerHTML = `<span style="color:#3fb950;font-weight:600">✅ PASSED</span> (${esc(j.reason)}) &mdash; Forwarded to Upstream DNS (${esc(j.upstream)})`;
    }
  } catch(e){
    testres.innerHTML = '<span style="color:#f85149">Test request failed</span>';
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
  if(!ip){alert('Please enter an IP address');return;}
  upmsg.textContent = 'Updating upstream DNS...';
  try {
    let r = await fetch('/setupstream?ip='+encodeURIComponent(ip)+'&port='+encodeURIComponent(port)+'&name='+encodeURIComponent(name),{headers:CSRF_HDRS});
    if(r.ok){
      upmsg.style.color='#3fb950';upmsg.textContent='✓ Upstream DNS updated successfully!';
      setTimeout(load, 400);
    } else {
      upmsg.style.color='#f85149';upmsg.textContent='✗ Failed: '+await r.text();
    }
  } catch(e){
    upmsg.style.color='#f85149';upmsg.textContent='✗ Network error';
  }
}

let inFlight=false,lastCustom='',lastAllow='',lastTop='';
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
    let stMsg=on?'Blocking active':(s.resumeIn>0?'Paused — resumes in '+s.resumeIn+'s':'Paused');
    if(blockstate.textContent!==stMsg)blockstate.textContent=stMsg;
    pausebtn.textContent=on?'Pause':'Resume';pausedur.style.display=on?'':'none';

    // Metrics cards
    v_blocked.textContent=fmt(s.blocked);
    v_allowed.textContent=fmt(s.allowed);
    let tot = (s.blocked||0) + (s.allowed||0);
    v_rate.textContent = tot>0 ? ((s.blocked*100/tot).toFixed(1)+'%') : '0.0%';
    v_domains.textContent=fmt(s.domains);
    v_clients.textContent=s.clients.length;
    v_wifi.textContent=s.rssi+' dBm';
    v_temp.textContent=s.temp+' °C';
    v_heap.textContent=Math.round(s.heap/1024)+' KB';
    v_psram.textContent=Math.round((s.psram||0)/1024)+' KB';
    v_uptime.textContent=s.uptime;

    // Weather Header
    if(s.weather && s.weather.valid){
      wthr.innerHTML = `🌤️ Taipei: <b>${s.weather.temp.toFixed(1)}°C</b> (${esc(s.weather.cond)}) &bull; Hum: ${s.weather.hum}%`;
    }

    // Upstream DNS Active Display
    if(s.upstream){
      activeup.textContent = `${s.upstream.name} (${s.upstream.ip}:${s.upstream.port})`;
      if(document.activeElement!=upip&&!upip.value)upip.value=s.upstream.ip;
      if(document.activeElement!=upport&&!upport.value)upport.value=s.upstream.port;
      if(document.activeElement!=upname&&!upname.value)upname.value=s.upstream.name;
    }

    // Clients Table (in-place update)
    let tb=ct.tBodies[0],cur=new Set();
    s.clients.forEach(c=>{
      cur.add(c.ip);let row=tb.querySelector(`tr[data-ip="${c.ip}"]`);
      if(!row){row=document.createElement('tr');row.dataset.ip=c.ip;
      row.innerHTML=`<td class=cip></td><td class=cmac></td><td class="b cblk"></td><td class="a calw"></td><td><button class=ban data-ip="${c.ip}"></button></td>`;
      tb.appendChild(row);}
      let ipH=esc(c.ip)+(c.banned?' <span class=tag style=color:#f85149>BANNED</span>':'');
      let tdIp=row.querySelector('.cip');if(tdIp.innerHTML!==ipH)tdIp.innerHTML=ipH;
      let tdMac=row.querySelector('.cmac');if(tdMac.textContent!==c.mac)tdMac.textContent=c.mac;
      let tdB=row.querySelector('.cblk'),bS=fmt(c.blocked);if(tdB.textContent!==bS)tdB.textContent=bS;
      let tdA=row.querySelector('.calw'),aS=fmt(c.allowed);if(tdA.textContent!==aS)tdA.textContent=aS;
      let btn=row.querySelector('.ban'),wT=c.banned?'Unban':'Ban';if(btn.textContent!==wT)btn.textContent=wT;
    });
    Array.from(tb.children).forEach(r=>{if(r.dataset.ip&&!cur.has(r.dataset.ip))r.remove()});

    // Query Logs Table
    if(s.logs && s.logs.length){
      let nowS = Math.floor(Date.now() / 1000);
      lt.tBodies[0].innerHTML = s.logs.map(l=>{
        let st = l.b ? '<span class=badge-blk>BLOCKED</span>' : '<span class=badge-alw>FORWARD</span>';
        let act = l.b ? `<button class="alwbtn" data-d="${esc(l.d)}" style="padding:2px 8px;font-size:11px">+Allow</button>`
                      : `<button class="blkbtn" data-d="${esc(l.d)}" style="padding:2px 8px;font-size:11px">+Block</button>`;
        return `<tr><td style="color:#8b949e">${l.t ? (l.t%86400<10?'0':'')+Math.floor((l.t%3600)/60)+'m '+(l.t%60)+'s' : 'just now'}</td><td>${esc(l.ip)}</td><td title="${esc(l.d)}">${esc(l.d)}</td><td>${st}</td><td>${act}</td></tr>`;
      }).join('');
    }

    // Top Blocked Table
    let topStr = JSON.stringify(s.topBlocked||[]);
    if(topStr !== lastTop){
      lastTop = topStr;
      if(s.topBlocked && s.topBlocked.length){
        tbt.tBodies[0].innerHTML = s.topBlocked.map((t,idx)=>`<tr><td><b>#${idx+1}</b></td><td title="${esc(t.domain)}">${esc(t.domain)}</td><td class=b>${fmt(t.count)}</td><td><button class=alwbtn data-d="${esc(t.domain)}" style="padding:2px 8px;font-size:11px">+Allow</button></td></tr>`).join('');
      } else {
        tbt.tBodies[0].innerHTML = '<tr><td colspan=4 style="color:#8b949e">No blocked queries recorded yet</td></tr>';
      }
    }

    // Allowlist Table
    let aStr = (s.allow||[]).join('|');
    if(aStr !== lastAllow){
      lastAllow = aStr;
      at.tBodies[0].innerHTML = (s.allow&&s.allow.length)?s.allow.map(d=>`<tr><td>${esc(d)}</td><td style="text-align:right"><button class=rmalwbtn data-d="${esc(d)}">remove</button></td></tr>`).join(''):'<tr><td style="color:#8b949e">none yet</td></tr>';
    }

    // Custom Block Table
    let cStr = (s.custom||[]).join('|');
    if(cStr !== lastCustom){
      lastCustom = cStr;
      cl.tBodies[0].innerHTML=(s.custom&&s.custom.length)?s.custom.map(d=>`<tr><td>${esc(d)}</td><td style="text-align:right"><button class=rmbtn data-d="${esc(d)}">remove</button></td></tr>`).join(''):'<tr><td style="color:#8b949e">none yet</td></tr>';
    }

    if(document.activeElement!=uurl)uurl.value=s.upurl||'';
    if(document.activeElement!=uiv)uiv.value=s.upiv||24;
    if(ustat.textContent!==(s.upstat||'—'))ustat.textContent=s.upstat||'—';
  }catch(_){}finally{inFlight=false;}
}

async function changePassword(){
  let u=newuser.value.trim(), c=curpass.value, p=newpass.value;
  if(!p || p.length < 4){ alert('新密碼長度至少需 4 個字元 (Password must be at least 4 characters)'); return; }
  pwdmsg.style.color='#8b949e';pwdmsg.textContent='Updating password...';
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

ct.addEventListener('click',e=>{if(e.target.classList.contains('ban'))fetch('/ban?ip='+e.target.dataset.ip,{headers:CSRF_HDRS}).then(load)});
cl.addEventListener('click',e=>{if(e.target.classList.contains('rmbtn'))fetch('/unblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});
at.addEventListener('click',e=>{if(e.target.classList.contains('rmalwbtn'))fetch('/unallow?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});

document.addEventListener('click',e=>{
  if(e.target.classList.contains('alwbtn')){
    fetch('/addallow?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load);
  } else if(e.target.classList.contains('blkbtn')){
    fetch('/addblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load);
  }
});

function saveUpd(){fetch('/setupdate?u='+encodeURIComponent(uurl.value.trim())+'&h='+(parseInt(uiv.value)||24),{headers:CSRF_HDRS}).then(load)}
function fetchNow(){ustat.textContent='fetching...';fetch('/fetchnow',{headers:CSRF_HDRS}).then(r=>r.text()).then(t=>{ustat.textContent=t;load()})}
function forgetWifi(){fetch('/forgetwifi',{headers:CSRF_HDRS}).then(r=>r.text()).then(t=>alert(t))}

fwf.onsubmit=async e=>{e.preventDefault();let f=fwb.files[0];if(!f)return;fwmsg.textContent='flashing '+(f.size/1048576).toFixed(2)+' MB...';
  let fd=new FormData();fd.append('f',f);
  try{let r=await fetch('/update',{method:'POST',headers:CSRF_HDRS,body:fd});fwmsg.textContent=r.ok?'✓ rebooting, reconnect in ~15s':'✗ '+await r.text();}
  catch(_){fwmsg.textContent='✓ rebooting, reconnect in ~15s';}};

upf.onsubmit=async e=>{e.preventDefault();let f=blf.files[0];if(!f)return;
  upmsg.textContent='uploading '+(f.size/1048576).toFixed(2)+' MB...';
  let fd=new FormData();fd.append('f',f);
  try{let r=await fetch('/upload',{method:'POST',headers:CSRF_HDRS,body:fd});upmsg.textContent=r.ok?'✓ updated':'✗ '+await r.text();}
  catch(_){upmsg.textContent='✗ upload failed';}
  blf.value='';setTimeout(load,600);};

load();setInterval(load,3000);
</script></body></html>)HTML";
