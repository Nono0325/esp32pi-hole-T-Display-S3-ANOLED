#pragma once
// Dashboard HTML for the T-Display-S3 AMOLED AdBlocker web UI.
// Built for rock-solid stability with zero layout shift, in-place DOM updates,
// tabular numbers, fixed column layouts, and smooth real-time refreshes.

const char PAGE[] PROGMEM = R"HTML(<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>T-Display-S3 AMOLED AdBlock</title><style>
*{box-sizing:border-box}
html,body{font:14px system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;margin:0;background:#0d1117;color:#c9d1d9;overflow-y:scroll}
header{background:#161b22;padding:14px 18px;border-bottom:1px solid #30363d}
h1{margin:0;font-size:18px}h1 span{color:#3fb950}.wrap{padding:16px;max-width:1000px;margin:auto}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(135px,1fr));gap:12px;margin-bottom:16px}
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:12px 14px;min-height:70px}
.card .v{font-size:20px;font-weight:600;font-variant-numeric:tabular-nums;line-height:1.2}
.card .l{color:#8b949e;font-size:12px;margin-top:4px}
table{width:100%;border-collapse:collapse;background:#161b22;border-radius:8px;overflow:hidden;margin-bottom:18px;table-layout:fixed}
th,td{padding:9px 12px;text-align:left;border-bottom:1px solid #21262d;font-size:13px;font-variant-numeric:tabular-nums;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
th{background:#21262d;color:#8b949e}tr:hover td{background:#1c2128}
.b{color:#f85149}.a{color:#3fb950}.tag{background:#30363d;border-radius:4px;padding:1px 6px;font-size:11px}
button{background:#21262d;color:#c9d1d9;border:1px solid #30363d;border-radius:5px;padding:5px 10px;cursor:pointer;font-size:13px}
button:hover{background:#30363d}.ban{color:#f85149}input{background:#0d1117;border:1px solid #30363d;color:#c9d1d9;border-radius:5px;padding:6px}
h2{font-size:14px;color:#8b949e;margin:18px 0 8px}
#blockbar{display:flex;align-items:center;gap:12px;margin-bottom:14px;padding:12px 14px;background:#161b22;border:1px solid #30363d;border-radius:8px;min-height:54px}
</style></head><body>
<header><h1>🛡️ T-Display-S3 AMOLED AdBlock <span id=host></span></h1></header><div class=wrap>
<div id=credwarn style="display:none;background:#3b1d1d;border:1px solid #f85149;color:#ffb3ae;border-radius:8px;padding:10px 14px;margin-bottom:14px;font-size:13px">
⚠️ <b>Default credentials in use.</b> WEB_PASS/OTA_PASS in <code>secrets.h</code> are still placeholder values. Set real values and reflash.
</div>
<div id=blockbar>
<span id=blockdot style="font-size:20px;line-height:1">🛡️</span><b id=blockstate style="flex:1" data-on=1>Blocking active</b>
<select id=pausedur style="background:#0d1117;border:1px solid #30363d;color:#c9d1d9;border-radius:5px;padding:5px"><option value=30>30s</option><option value=300 selected>5 min</option><option value=1800>30 min</option><option value=0>until I re-enable</option></select>
<button id=pausebtn onclick=togglePause()>Pause</button>
<button id=flipbtn onclick=flipScreen() style="margin-left:6px" title="Flip Screen 180°">🔄 Flip 180°</button></div>
<div class=cards id=sys>
<div class=card><div class="v b" id=v_blocked>0</div><div class=l>Total blocked</div></div>
<div class=card><div class="v a" id=v_allowed>0</div><div class=l>Total allowed</div></div>
<div class=card><div class=v id=v_domains>0 domains</div><div class=l>Blocklist</div></div>
<div class=card><div class=v id=v_clients>0</div><div class=l>Clients</div></div>
<div class=card><div class=v id=v_wifi>--</div><div class=l>WiFi</div></div>
<div class=card><div class=v id=v_temp>--</div><div class=l>Temp</div></div>
<div class=card><div class=v id=v_heap>--</div><div class=l>Free RAM</div></div>
<div class=card><div class=v id=v_psram>--</div><div class=l>PSRAM</div></div>
<div class=card><div class=v id=v_uptime>--</div><div class=l>Uptime</div></div>
</div>
<h2>CLIENTS</h2>
<table id=ct>
<colgroup><col style="width:26%"><col style="width:26%"><col style="width:18%"><col style="width:18%"><col style="width:12%"></colgroup>
<thead><tr><th>Client</th><th>MAC</th><th>Blocked</th><th>Allowed</th><th>Action</th></tr></thead>
<tbody></tbody></table>
<h2>CUSTOM BLOCKED DOMAINS</h2>
<div style="margin-bottom:8px"><input id=dom placeholder="ads.example.com" size=30><button onclick=addDom()>Block domain</button></div>
<table id=cl><colgroup><col style="width:80%"><col style="width:20%"></colgroup><tbody></tbody></table>
<h2>BLOCKLIST &mdash; UPLOAD</h2>
<form id=upf style="margin-bottom:6px"><input type=file id=blf accept=.bin><button>Upload blocklist</button> <span id=upmsg style="color:#8b949e"></span></form>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">build <code>blocklist.bin</code> with <code>tools/build_blocklist.py</code>, then upload here &mdash; no USB</div>
<h2>BLOCKLIST &mdash; REMOTE AUTO-UPDATE</h2>
<div style="margin-bottom:6px"><input id=uurl placeholder="https://host/blocklist.bin" size=40> every <input id=uiv size=2 value=24>h
<button onclick=saveUpd()>Save</button> <button onclick=fetchNow()>Fetch now</button></div>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">device pulls a prebuilt <code>blocklist.bin</code> on a schedule (e.g. a GitHub release asset). last: <span id=ustat>&mdash;</span></div>
<h2>FIRMWARE &mdash; OTA UPDATE</h2>
<form id=fwf style="margin-bottom:6px"><input type=file id=fwb accept=.bin><button>Flash firmware</button> <span id=fwmsg style="color:#8b949e"></span></form>
<div style="color:#8b949e;font-size:12px;margin-bottom:18px">upload <code>.pio/build/t-display-s3-amoled/firmware.bin</code> &mdash; device verifies it and reboots into it</div>
<h2>ADMIN &amp; PASSWORD</h2>
<div style="background:#161b22;border:1px solid #30363d;border-radius:8px;padding:14px;margin-bottom:18px">
  <div style="display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:8px">
    <input id=newuser placeholder="Username (default: admin)" size=18>
    <input type=password id=curpass placeholder="Current password" size=18>
    <input type=password id=newpass placeholder="New password (min 4)" size=18>
    <button onclick="changePassword()" style="background:#238636;color:#fff;border-color:#2ea043">Save Password</button>
  </div>
  <div id=pwdmsg style="font-size:13px;min-height:18px;color:#8b949e"></div>
  <div style="color:#8b949e;font-size:12px;margin-top:4px">Credentials are stored in ESP32 flash memory for dashboard control and OTA firmware updates.</div>
</div>
<h2>WIFI</h2>
<div style="margin-bottom:18px"><button onclick="if(confirm('Forget saved WiFi and reboot into the setup portal?'))forgetWifi()">Forget WiFi</button></div>
</div><script>
function fmt(n){return (n||0).toLocaleString()}
function esc(s){return String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
const CSRF_HDRS={'X-Requested-With':'c3-adblock'};
function togglePause(){fetch(blockstate.dataset.on=='1'?'/pause?s='+pausedur.value:'/resume',{headers:CSRF_HDRS}).then(load);}
function flipScreen(){fetch('/flip',{headers:CSRF_HDRS});}
let inFlight=false,lastCustom='';
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
v_blocked.textContent=fmt(s.blocked);
v_allowed.textContent=fmt(s.allowed);
v_domains.textContent=fmt(s.domains)+' domains';
v_clients.textContent=s.clients.length;
v_wifi.textContent=s.rssi+' dBm';
v_temp.textContent=s.temp+' °C';
v_heap.textContent=Math.round(s.heap/1024)+' KB';
v_psram.textContent=Math.round((s.psram||0)/1024)+' KB';
v_uptime.textContent=s.uptime;
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
let cStr=(s.custom||[]).join('|');
if(cStr!==lastCustom){lastCustom=cStr;
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
pwdmsg.style.color='#8b949e';
pwdmsg.textContent='Updating password...';
try {
let r = await fetch('/setpassword?newpass=' + encodeURIComponent(p) + '&curpass=' + encodeURIComponent(c) + (u ? ('&newuser=' + encodeURIComponent(u)) : ''), {
method: 'POST',
headers: CSRF_HDRS
});
if(r.ok) {
pwdmsg.style.color='#3fb950';
pwdmsg.textContent='✓ 密碼修改成功！';
curpass.value='';
newpass.value='';
setTimeout(()=>{ alert('密碼修改成功！日後請使用新密碼進行管理與更新。'); }, 50);
} else {
pwdmsg.style.color='#f85149';
let err = await r.text();
pwdmsg.textContent='✗ 修改失敗: ' + err;
}
} catch(e) {
pwdmsg.style.color='#f85149';
pwdmsg.textContent='✗ 請求失敗，請檢查網路連線';
}
}
function addDom(){let d=dom.value.trim();if(d){fetch('/addblock?d='+encodeURIComponent(d),{headers:CSRF_HDRS}).then(()=>{dom.value='';load()})}}
ct.addEventListener('click',e=>{if(e.target.classList.contains('ban'))fetch('/ban?ip='+e.target.dataset.ip,{headers:CSRF_HDRS}).then(load)});
cl.addEventListener('click',e=>{if(e.target.classList.contains('rmbtn'))fetch('/unblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});
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
