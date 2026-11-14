import json, urllib.request, urllib.parse, re, os, io, sys, subprocess

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
cfg = os.path.expanduser(r'~\.config\clash\config.yaml')
txt = open(cfg, encoding='utf-8').read()
port = re.search(r'external-controller:\s*127\.0\.0\.1:(\d+)', txt).group(1)
secret = re.search(r'secret:\s*(\S+)', txt).group(1)


def api(path, timeout=15):
    req = urllib.request.Request(f'http://127.0.0.1:{port}' + path,
                                 headers={'Authorization': f'Bearer {secret}'})
    return json.loads(urllib.request.urlopen(req, timeout=timeout).read())


p = api('/proxies')['proxies']
G = '🔰 选择节点'
print('当前节点 :', p[G]['now'])
print('漏网之鱼 :', p['🐟 漏网之鱼']['now'])
print('代理组数 :', sum(1 for v in p.values() if v.get('type') in ('Selector', 'URLTest')))

UA = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36'
print('\n=== 最终验收（经系统代理 127.0.0.1:7890）===')
sites = [
    ('Google', 'http://www.gstatic.com/generate_204', '204'),
    ('YouTube', 'https://www.youtube.com/', '200'),
    ('ChatGPT', 'https://chatgpt.com/', '200'),
    ('OpenAI API', 'https://api.openai.com/v1/models', '401'),
    ('Wikipedia', 'https://en.wikipedia.org/', '200'),
    ('GitHub', 'https://github.com/', '200'),
]
passed = 0
for label, url, expect in sites:
    r = subprocess.run(['curl', '-s', '-o', 'NUL', '-w', '%{http_code} %{time_total}',
                        '--max-time', '15', '-x', 'http://127.0.0.1:7890', '-A', UA, url],
                       capture_output=True, text=True, timeout=20)
    out = r.stdout.strip()
    code = out.split()[0] if out else '000'
    ok = code == expect
    passed += ok
    print(f"  {'PASS' if ok else 'FAIL'}  {label:<12} HTTP {out}")
print(f'\n通过 {passed}/{len(sites)}')

r = subprocess.run(['curl', '-s', '--max-time', '12', '-x', 'http://127.0.0.1:7890',
                    'https://chatgpt.com/cdn-cgi/trace'], capture_output=True, text=True, timeout=18)
info = dict(l.split('=', 1) for l in r.stdout.strip().splitlines() if '=' in l)
print(f"\n出口 IP : {info.get('ip','?')}   地区 : {info.get('loc','?')}")
