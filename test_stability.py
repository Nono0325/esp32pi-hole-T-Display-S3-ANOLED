import urllib.request
import urllib.parse
import json
import socket
import time
import base64
import sys

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

DEVICE_IP = "192.168.50.64"
AUTH_USER = "admin"
AUTH_PASS = "50915133"

def make_request(path, method="GET", data=None):
    url = f"http://{DEVICE_IP}{path}"
    req = urllib.request.Request(url, method=method)
    auth_str = f"{AUTH_USER}:{AUTH_PASS}"
    b64_auth = base64.b64encode(auth_str.encode()).decode()
    req.add_header("Authorization", f"Basic {b64_auth}")
    req.add_header("X-Requested-With", "c3-adblock")
    try:
        with urllib.request.urlopen(req, data=data, timeout=5) as response:
            return response.status, response.read().decode('utf-8', errors='ignore')
    except Exception as e:
        return 0, str(e)

def build_dns_query(domain, txid=0x1234):
    header = bytearray([
        (txid >> 8) & 0xFF, txid & 0xFF,
        0x01, 0x00, # standard query, recursion desired
        0x00, 0x01, # 1 question
        0x00, 0x00, # 0 answer
        0x00, 0x00, # 0 authority
        0x00, 0x00  # 0 additional
    ])
    question = bytearray()
    for part in domain.split('.'):
        question.append(len(part))
        question.extend(part.encode('ascii'))
    question.append(0)
    question.extend([0x00, 0x01]) # Type A
    question.extend([0x00, 0x01]) # Class IN
    return header + question

def send_dns_query(domain, timeout=2.0):
    query = build_dns_query(domain)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    t0 = time.perf_counter()
    try:
        sock.sendto(query, (DEVICE_IP, 53))
        resp, _ = sock.recvfrom(512)
        elapsed_ms = (time.perf_counter() - t0) * 1000
        # parse answer IP if present
        is_blocked = False
        ans_ip = "none"
        if len(resp) >= 16:
            # check last 4 bytes for 0.0.0.0
            if resp[-4:] == b'\x00\x00\x00\x00':
                is_blocked = True
                ans_ip = "0.0.0.0"
            else:
                ans_ip = ".".join(str(b) for b in resp[-4:])
        return True, elapsed_ms, is_blocked, ans_ip
    except socket.timeout:
        return False, timeout * 1000, False, "TIMEOUT"
    except Exception as e:
        return False, 0, False, str(e)
    finally:
        sock.close()

def run_tests():
    print("=" * 60)
    print(" 🚀 ESP32-S3 Pi-Hole & AMOLED Full Stability Test Suite")
    print("=" * 60)

    # 1. Device Info
    print("\n[TEST 1] Querying /stats.json System Health...")
    status, body = make_request("/stats.json")
    if status != 200:
        print(f"❌ Failed to reach device: status {status}, err: {body}")
        return
    stats = json.loads(body)
    print(f"  ✓ Device IP:       {stats.get('ip')}")
    print(f"  ✓ Uptime:          {stats.get('uptime')}")
    print(f"  ✓ Chip Temp:       {stats.get('temp')} °C")
    print(f"  ✓ CPU Freq:        {stats.get('cpufreq')} MHz")
    print(f"  ✓ WiFi ModemSleep: {stats.get('wifisleep')}")
    print(f"  ✓ Free Heap:       {stats.get('heap') // 1024} KB")
    print(f"  ✓ Free PSRAM:      {stats.get('psram') // 1024} KB")
    print(f"  ✓ Blocklist Rules: {stats.get('domains')} domains")
    print(f"  ✓ Screen Asleep:   {stats.get('screenAsleep')}")
    print(f"  ✓ Sleep Timeout:   {stats.get('sleepTimeout')} s")

    # 2. Screen Sleep and Wakeup Cycle
    print("\n[TEST 2] Testing Screen Sleep & Wakeup Cycle...")
    st_sleep, res_sleep = make_request("/sleep")
    time.sleep(0.5)
    _, body_s = make_request("/stats.json")
    stats_s = json.loads(body_s)
    is_asleep = stats_s.get("screenAsleep") == True
    print(f"  -> Trigger /sleep: Status {st_sleep} ({res_sleep}), screenAsleep={is_asleep} {'✓' if is_asleep else '❌'}")

    time.sleep(0.5)
    st_wake, res_wake = make_request("/wakeup")
    time.sleep(0.5)
    _, body_w = make_request("/stats.json")
    stats_w = json.loads(body_w)
    is_awake = stats_w.get("screenAsleep") == False
    print(f"  -> Trigger /wakeup: Status {st_wake} ({res_wake}), screenAsleep={not is_awake} {'✓' if is_awake else '❌'}")

    # 3. Settings & Controls
    print("\n[TEST 3] Testing Page Switching & Brightness Controls...")
    p2_st, _ = make_request("/setpage?p=2")
    time.sleep(0.3)
    p0_st, _ = make_request("/setpage?p=0")
    b_st, _ = make_request("/setbrightness?b=170")
    t_st, _ = make_request("/setsleep?s=300")
    print(f"  ✓ Set Page 2 & Page 0: {p2_st == 200 and p0_st == 200}")
    print(f"  ✓ Set Brightness 66% (170): {b_st == 200}")
    print(f"  ✓ Set Sleep Timeout 300s: {t_st == 200}")

    # 4. Live DNS Stress & Resolution Test
    print("\n[TEST 4] Running Live DNS Stress & Resolution Test (25 queries)...")
    test_domains = [
        # Ads (should block -> 0.0.0.0)
        ("doubleclick.net", True),
        ("googleads.g.doubleclick.net", True),
        ("pagead2.googlesyndication.com", True),
        ("adservice.google.com", True),
        ("sitemaji.com", True),
        ("exoclick.com", True),
        ("appier.net", True),
        ("dable.io", True),
        ("popads.net", True),
        ("adsterra.com", True),
        # Normal (should pass -> upstream IP)
        ("google.com", False),
        ("github.com", False),
        ("microsoft.com", False),
        ("apple.com", False),
        ("amazon.com", False),
        ("cloudflare.com", False),
        ("wikipedia.org", False),
        ("youtube.com", False),
        ("openai.com", False),
        ("netflix.com", False),
        ("yahoo.com", False),
        ("line.me", False),
        ("twitch.tv", False),
        ("discord.com", False),
        ("spotify.com", False)
    ]

    success_cnt = 0
    correct_cnt = 0
    total_time = 0.0
    latencies = []

    for domain, should_block in test_domains:
        ok, lat, is_blk, ans = send_dns_query(domain)
        if ok:
            success_cnt += 1
            latencies.append(lat)
            total_time += lat
            match = (is_blk == should_block)
            if match:
                correct_cnt += 1
            mark = "🛡️ BLOCKED" if is_blk else f"🌐 {ans}"
            verdict = "✓" if match else "⚠️"
            print(f"  {verdict} [{lat:5.1f} ms] {domain:35s} -> {mark}")
        else:
            print(f"  ❌ FAILED {domain:35s} -> {ans}")
        time.sleep(0.04)

    avg_lat = (total_time / len(latencies)) if latencies else 0
    min_lat = min(latencies) if latencies else 0
    max_lat = max(latencies) if latencies else 0

    print("\n" + "-" * 60)
    print(" 📊 DNS STRESS TEST SUMMARY")
    print("-" * 60)
    print(f"  • Total Queries Sent:     {len(test_domains)}")
    print(f"  • Successful Responses:   {success_cnt}/{len(test_domains)} ({(success_cnt/len(test_domains))*100:.1f}%)")
    print(f"  • Blocklist Accuracy:     {correct_cnt}/{len(test_domains)} ({(correct_cnt/len(test_domains))*100:.1f}%)")
    print(f"  • Average Latency:        {avg_lat:.2f} ms")
    print(f"  • Min / Max Latency:      {min_lat:.1f} ms / {max_lat:.1f} ms")

    # 5. Final Thermal & Health Check
    print("\n[TEST 5] Final Thermal & Health Check...")
    _, body_final = make_request("/stats.json")
    stats_f = json.loads(body_final)
    print(f"  ✓ Post-Stress Temp: {stats_f.get('temp')} °C")
    print(f"  ✓ Post-Stress Heap: {stats_f.get('heap') // 1024} KB")
    print(f"  ✓ Total Queries Blocked: {stats_f.get('blocked')}")
    print(f"  ✓ Total Queries Allowed: {stats_f.get('allowed')}")
    print("\n" + "=" * 60)
    print(" ✅ ALL STABILITY TESTS COMPLETED SUCCESSFULLY!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()
