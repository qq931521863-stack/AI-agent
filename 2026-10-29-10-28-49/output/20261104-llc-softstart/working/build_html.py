# -*- coding: utf-8 -*-
import re, html, os, sys

BASE = r"C:\Users\zhizh\WorkBuddy AI\2026-10-29-10-28-49\output\20261104-llc-softstart"
SRC = sys.argv[1] if len(sys.argv) > 1 else BASE + r"\stage1\final_draft.md"
OUT = sys.argv[2] if len(sys.argv) > 2 else BASE + r"\stage2\formatted-半桥LLC软启动.html"
EDITION = sys.argv[3] if len(sys.argv) > 3 else "第二版（模型校核后修订）"

raw = open(SRC, encoding="utf-8").read()
lines = raw.split("\n")

def inline(t):
    t = html.escape(t, quote=False)
    codes = []
    def stash(m):
        codes.append(m.group(1))
        return "\x00%d\x00" % (len(codes) - 1)
    t = re.sub(r"`([^`]+)`", stash, t)
    t = re.sub(r"\*\*(.+?)\*\*", r"<strong>\1</strong>", t)
    t = re.sub(r"(?<![\*\w])\*([^\*\n]+?)\*(?![\*\w])", r"<em>\1</em>", t)
    def back(m):
        return '<code class="ic">%s</code>' % codes[int(m.group(1))]
    t = re.sub("\x00(\\d+)\x00", back, t)
    return t

blocks = []
i = 0
n = len(lines)
title_text = ""
while i < n:
    s = lines[i].strip()
    if not s:
        i += 1
        continue
    if s.startswith("```"):
        i += 1
        buf = []
        while i < n and not lines[i].strip().startswith("```"):
            buf.append(lines[i])
            i += 1
        i += 1
        blocks.append(("code", "\n".join(buf)))
        continue
    m = re.match(r"^(#{1,4})\s+(.*)$", s)
    if m:
        blocks.append(("h%d" % len(m.group(1)), m.group(2).strip()))
        i += 1
        continue
    if re.match(r"^-{3,}$", s):
        blocks.append(("hr", ""))
        i += 1
        continue
    if s.startswith("|"):
        rows = []
        while i < n and lines[i].strip().startswith("|"):
            rows.append(lines[i].strip())
            i += 1
        blocks.append(("table", rows))
        continue
    if s.startswith(">"):
        buf = []
        while i < n and lines[i].strip().startswith(">"):
            buf.append(lines[i].strip().lstrip(">").strip())
            i += 1
        blocks.append(("quote", " ".join(buf)))
        continue
    if re.match(r"^[-*]\s+", s):
        items = []
        while i < n and re.match(r"^\s*[-*]\s+", lines[i]):
            items.append(re.sub(r"^\s*[-*]\s+", "", lines[i]).strip())
            i += 1
        blocks.append(("ul", items))
        continue
    if re.match(r"^\d+\.\s+", s):
        items = []
        while i < n and re.match(r"^\s*\d+\.\s+", lines[i]):
            items.append(re.sub(r"^\s*\d+\.\s+", "", lines[i]).strip())
            i += 1
        blocks.append(("ol", items))
        continue
    buf = [s]
    i += 1
    while i < n:
        t = lines[i].strip()
        if not t:
            break
        if re.match(r"^(#{1,4}\s|```|\||>|[-*]\s|\d+\.\s|-{3,}$)", t):
            break
        buf.append(t)
        i += 1
    blocks.append(("p", " ".join(buf)))

for k, v in blocks:
    if k == "h1":
        title_text = v
        break

if "：" in title_text:
    ct, cs = title_text.split("：", 1)
else:
    ct, cs = title_text, ""

body = []
toc = []
sec = 0
cur = None
for kind, val in blocks:
    if kind == "h1":
        continue
    elif kind == "h2":
        sec += 1
        hid = "section-%d" % sec
        body.append('<h1 id="%s">%s</h1>' % (hid, inline(val)))
        cur = [val, hid, []]
        toc.append(cur)
    elif kind == "h3":
        sid = "s-%d-%d" % (sec, len(cur[2]) + 1)
        body.append('<h2 id="%s">%s</h2>' % (sid, inline(val)))
        if cur is not None:
            cur[2].append([val, sid])
    elif kind == "h4":
        body.append("<h3>%s</h3>" % inline(val))
    elif kind == "p":
        body.append("<p>%s</p>" % inline(val))
    elif kind == "quote":
        body.append('<p class="quote">&nbsp;&nbsp;%s</p>' % inline(val))
    elif kind == "ul":
        body.append("<ul>" + "".join("<li>%s</li>" % inline(x) for x in val) + "</ul>")
    elif kind == "ol":
        body.append("<ol>" + "".join("<li>%s</li>" % inline(x) for x in val) + "</ol>")
    elif kind == "code":
        body.append('<pre class="cb">%s</pre>' % html.escape(val, quote=False))
    elif kind == "hr":
        body.append('<p class="hr"></p>')
    elif kind == "table":
        rows = val
        cells = []
        for r in rows:
            if re.match(r"^\|[\s:\-\|]+\|?$", r):
                continue
            rr = r.replace("\\|", "\x01")
            parts = [c.strip().replace("\x01", "|") for c in rr.strip().strip("|").split("|")]
            cells.append(parts)
        if not cells:
            continue
        out = ['<table>']
        out.append("<thead><tr>" + "".join("<th>%s</th>" % inline(c) for c in cells[0]) + "</tr></thead>")
        out.append("<tbody>")
        for r in cells[1:]:
            out.append("<tr>" + "".join("<td>%s</td>" % inline(c) for c in r) + "</tr>")
        out.append("</tbody></table>")
        body.append("".join(out))

toc_html = ['<nav class="doc-toc" aria-label="文档目录">', '<p class="toc-title">目 录</p>', '<ol class="toc-list">']
for t, hid, subs in toc:
    if subs:
        toc_html.append('<li><a href="#%s">%s</a><ol>' % (hid, html.escape(t, quote=False)))
        for st, sid in subs:
            toc_html.append('<li><a href="#%s">%s</a></li>' % (sid, html.escape(st, quote=False)))
        toc_html.append("</ol></li>")
    else:
        toc_html.append('<li><a href="#%s">%s</a></li>' % (hid, html.escape(t, quote=False)))
toc_html.append("</ol></nav>")

CSS = """
  :root{
    --page-content-width: 15.6cm;
    --ff-body: "宋体", SimSun, serif;
    --ff-head: "黑体", SimHei, sans-serif;
    --ff-mono: "Consolas", "Courier New", monospace;
    --fs-body: 12pt;
    --fs-h1: 19pt;
    --fs-h2: 13.5pt;
    --fs-h3: 12pt;
    --fs-table: 9.5pt;
    --fs-code: 9pt;
    --fs-inline: 9.5pt;
    --fs-cover: 30pt;
    --fs-coversub: 15pt;
    --fs-eyebrow: 10pt;
    --fs-toc: 11pt;
    --fs-toc2: 10.5pt;
    --fs-quote: 11pt;
    --fs-meta: 10pt;
    --color-primary: #14375E;
    --color-text: #1a1a1a;
    --color-muted: #7a8a9a;
    --color-sub: #3d5a78;
    --color-border: #c8d0da;
    --color-hr: #dde3ea;
    --color-bg: #ffffff;
    --color-th-bg: #eaf1f8;
    --color-code-bg: #f7f9fb;
    --color-inline-bg: #f0f3f7;
    --color-inline-fg: #0b4f8a;
    --color-quote-bg: #f2f5f9;
    --color-quote-fg: #33475b;
    --color-accent: #2E75B6;
    --spacing-paragraph: 0.55em;
    --spacing-block: 0.95em;
    --spacing-section: 1.6em;
    --spacing-small: 0.28em;
    --spacing-mid: 0.7em;
    --spacing-list: 2.2em;
    --spacing-tocsub: 1.8em;
    --spacing-pad-code: 8px 10px;
    --spacing-pad-cell: 5px 7px;
    --spacing-pad-quote: 9px 12px;
    --spacing-pb-h1: 0.22em;
  }
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:var(--ff-body);font-size:var(--fs-body);line-height:1.75;color:var(--color-text);
       background:var(--color-bg);max-width:var(--page-content-width);margin-left:auto;margin-right:auto;padding:0}
  p{font-family:var(--ff-body);font-size:var(--fs-body);line-height:1.75;color:var(--color-text);
    text-indent:2em;text-align:justify;margin-bottom:var(--spacing-paragraph)}
  h1{font-family:var(--ff-head);font-size:var(--fs-h1);font-weight:700;color:var(--color-primary);
     line-height:1.35;margin-top:var(--spacing-section);margin-bottom:var(--spacing-mid);
     border-bottom:1.5px solid var(--color-primary);padding-bottom:var(--spacing-pb-h1)}
  h2{font-family:var(--ff-head);font-size:var(--fs-h2);font-weight:700;color:var(--color-primary);
     line-height:1.4;margin-top:var(--spacing-section);margin-bottom:var(--spacing-paragraph)}
  h3{font-family:var(--ff-head);font-size:var(--fs-h3);font-weight:700;color:var(--color-text);
     line-height:1.4;margin-top:var(--spacing-mid);margin-bottom:var(--spacing-small)}
  ul,ol{padding-left:var(--spacing-list);margin-bottom:var(--spacing-mid)}
  li{font-family:var(--ff-body);font-size:var(--fs-body);line-height:1.7;margin-bottom:var(--spacing-small)}
  strong{font-family:var(--ff-head);font-weight:700;color:var(--color-primary)}
  table{border-collapse:collapse;width:100%;margin:var(--spacing-paragraph) 0 var(--spacing-block) 0;
        font-family:var(--ff-body);font-size:var(--fs-table)}
  th,td{border:0.5px solid var(--color-border);padding:var(--spacing-pad-cell);text-align:left;
        vertical-align:top;line-height:1.5}
  th{background:var(--color-th-bg);font-weight:700;color:var(--color-primary)}
  pre.cb{font-family:var(--ff-mono);font-size:var(--fs-code);line-height:1.5;
         background:var(--color-code-bg);border-left:3px solid var(--color-accent);
         padding:var(--spacing-pad-code);margin:var(--spacing-paragraph) 0 var(--spacing-block) 0;
         white-space:pre-wrap;color:var(--color-text);text-indent:0}
  code.ic{font-family:var(--ff-mono);font-size:var(--fs-inline);color:var(--color-inline-fg);
          background:var(--color-inline-bg)}
  p.quote{border-left:3px solid var(--color-accent);background:var(--color-quote-bg);
          padding:var(--spacing-pad-quote);text-indent:0;font-size:var(--fs-quote);
          color:var(--color-quote-fg);line-height:1.7;margin:var(--spacing-block) 0}
  p.hr{border-bottom:0.5px solid var(--color-hr);margin:var(--spacing-section) 0}
  .c-eyebrow{text-align:center;text-indent:0;font-family:var(--ff-head);font-size:var(--fs-eyebrow);
             color:var(--color-muted);margin-top:var(--spacing-section);margin-bottom:var(--spacing-mid)}
  .c-title{text-align:center;text-indent:0;font-family:var(--ff-head);font-size:var(--fs-cover);
           font-weight:700;color:var(--color-primary);line-height:1.3;margin-bottom:var(--spacing-small)}
  .c-sub{text-align:center;text-indent:0;font-family:var(--ff-head);font-size:var(--fs-coversub);
         color:var(--color-sub);line-height:1.5;margin-bottom:var(--spacing-section)}
  table.c-meta{width:68%;margin:0 auto;font-size:var(--fs-meta)}
  table.c-meta td{border:none;border-bottom:0.5px solid var(--color-hr);padding:var(--spacing-pad-cell)}
  table.c-meta td.k{color:var(--color-muted);width:32%}
  .doc-toc{padding:0;background:transparent}
  .toc-title{text-align:center;text-indent:0;font-family:var(--ff-head);font-size:var(--fs-h2);
             color:var(--color-primary);margin-bottom:var(--spacing-mid)}
  .toc-list,.toc-list ol{list-style:none;list-style-type:none;padding-left:0;margin:0}
  .toc-list>li{font-family:var(--ff-head);font-size:var(--fs-toc);font-weight:700;
               color:var(--color-primary);line-height:1.95;margin-bottom:0;text-indent:0}
  .toc-list ol{padding-left:var(--spacing-tocsub)}
  .toc-list ol li{font-family:var(--ff-body);font-size:var(--fs-toc2);font-weight:400;
                  color:var(--color-text);line-height:1.75;margin-bottom:0;text-indent:0}
  .toc-list a{color:inherit;text-decoration:none}
  @page { @bottom-center { content: counter(page); } }
  @page cover { @bottom-center { content: none; } }
  section[role="cover"] { page: cover; }
"""

H = []
H.append('<!DOCTYPE html>')
H.append('<html lang="zh-CN">')
H.append('<head>')
H.append('<meta charset="UTF-8">')
H.append('<meta name="docx-page-size" content="A4">')
H.append('<title>%s</title>' % html.escape(title_text, quote=False))
H.append('<style>%s</style>' % CSS)
H.append('</head>')
H.append('<body>')
H.append('<section role="cover">')
H.append('<p class="c-eyebrow">电 力 电 子 · 设 计 笔 记</p>')
H.append('<p class="c-title">%s</p>' % html.escape(ct, quote=False))
H.append('<p class="c-sub">%s</p>' % html.escape(cs, quote=False))
H.append('<table class="c-meta"><tbody>')
H.append('<tr><td class="k">文档类型</td><td>技术原理与工程实践</td></tr>')
H.append('<tr><td class="k">目标读者</td><td>从事数字控制 LLC 的电源工程师</td></tr>')
H.append('<tr><td class="k">前置知识</td><td>半桥拓扑、谐振腔、PI 调节器、STM32 高分辨率定时器</td></tr>')
H.append('<tr><td class="k">样本工程</td><td>1600 W 充电机，1650 W/30 A，输出 0~58 V</td></tr>')
H.append('<tr><td class="k">版本</td><td>%s</td></tr>' % html.escape(EDITION or "第二版（模型校核后修订）", quote=False))
H.append('<tr><td class="k">编制日期</td><td>2026 年 11 月</td></tr>')
H.append('</tbody></table>')
H.append('</section>')
H.append('<section role="cover">')
H.extend(toc_html)
H.append('</section>')
H.append('<section role="body" data-page-restart="1">')
H.extend(body)
H.append('</section>')
H.append('</body>')
H.append('</html>')

os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, "w", encoding="utf-8", newline="\n").write("\n".join(H))
print("OK", OUT)
print("h1:", len(toc), "h2:", sum(len(t[2]) for t in toc), "chars:", len("\n".join(H)))
