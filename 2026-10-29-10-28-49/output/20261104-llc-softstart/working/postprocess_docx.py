# -*- coding: utf-8 -*-
"""docx 排版后处理：转换器不做的三件事

1. 表格行禁止跨页断行（否则会出现页首半行残片）
2. 表头行跨页重复
3. 表格宽度改为版心 100%，四列表的首列（序号列）压到 8%
4. 表格后第一段补 180 twips 上间距（否则正文贴着表格边框）

用法： python postprocess_docx.py <docx 路径>
"""
import re
import shutil
import sys
import zipfile


def fix_tables(doc):
    """cantSplit + 表头重复"""
    def one(m):
        t = m.group(0)
        cnt = [0]

        def row(rm):
            cnt[0] += 1
            head = "<w:tblHeader/>" if cnt[0] == 1 else ""
            return rm.group(0).replace(
                "<w:trPr/>", "<w:trPr>%s<w:cantSplit/></w:trPr>" % head, 1)

        return re.sub(r"<w:trPr/>", row, t)

    return re.sub(r"<w:tbl>.*?</w:tbl>", one, doc, flags=re.S)


def fix_width(doc):
    """表宽 100%，四列表首列压窄"""
    doc = re.sub(r'<w:tblW w:w="\d+" w:type="dxa"/>',
                 '<w:tblW w:w="5000" w:type="pct"/>', doc)

    def one(m):
        t = m.group(0)
        cols = re.findall(r'<w:gridCol w:w="(\d+)"/>', t)
        if len(cols) != 4:
            return t
        tot = sum(int(c) for c in cols)
        rest = sum(int(c) for c in cols[1:]) or 1
        new = [int(tot * 0.08)] + [int(tot * 0.92 * int(c) / rest) for c in cols[1:]]
        it = iter(new)
        return re.sub(r'<w:gridCol w:w="\d+"/>',
                      lambda _: '<w:gridCol w:w="%d"/>' % next(it), t)

    return re.sub(r"<w:tbl>.*?</w:tbl>", one, doc, flags=re.S)


def fix_gap(doc):
    """表格后第一段补上间距"""
    def one(m):
        return m.group(0).replace('<w:spacing w:before="0"',
                                  '<w:spacing w:before="180"', 1)

    return re.sub(r"</w:tbl>(?:(?!<w:p ).){0,80}?<w:p [^>]*>\s*<w:pPr>.*?</w:pPr>",
                  one, doc, flags=re.S)


def fix_keep_code(doc):
    """代码块（单行表）前的段落加 keepNext，避免引言与代码块被分页切开"""
    starts, depth, st = [], 0, None
    for m in re.finditer(r"<w:tbl[ >]|</w:tbl>", doc):
        if m.group(0).startswith("<w:tbl"):
            if depth == 0:
                st = m.start()
            depth += 1
        else:
            depth -= 1
            if depth == 0:
                blk = doc[st:m.end()]
                if len(re.findall(r"<w:tr[ >]", blk)) == 1:
                    starts.append(st)
    for s in reversed(starts):
        p = doc.rfind("<w:p ", 0, s)
        if p < 0:
            continue
        pp = doc.find("<w:pPr>", p)
        if pp < 0 or pp > s:
            continue
        if "keepNext" not in doc[pp:pp + 200]:
            doc = doc[:pp + 7] + "<w:keepNext/>" + doc[pp + 7:]
    return doc


A4 = ('<w:pgSz w:w="11906" w:h="16838"/>'
      '<w:pgMar w:top="1440" w:right="1800" w:bottom="1440" w:left="1800"'
      ' w:header="851" w:footer="992" w:gutter="0"/>')


def fix_pagesize(doc):
    """最后一个节常常没有 pgSz，会继承 Word 默认的 Letter 纸张"""
    def one(m):
        blk = m.group(0)
        if "<w:pgSz" in blk:
            return blk
        add = A4 if "<w:pgMar" not in blk else '<w:pgSz w:w="11906" w:h="16838"/>'
        return blk.replace("</w:sectPr>", add + "</w:sectPr>", 1)

    return re.sub(r"<w:sectPr.*?</w:sectPr>", one, doc, flags=re.S)


def run(path):
    z = zipfile.ZipFile(path)
    doc = z.read("word/document.xml").decode("utf-8")
    n0 = doc.count("<w:cantSplit/>")
    doc = fix_pagesize(doc)
    doc = fix_tables(doc)
    doc = fix_width(doc)
    doc = fix_gap(doc)
    doc = fix_keep_code(doc)

    tmp = path + ".tmp"
    zout = zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED)
    for it in z.infolist():
        data = z.read(it.filename)
        if it.filename == "word/document.xml":
            data = doc.encode("utf-8")
        zout.writestr(it, data)
    zout.close()
    z.close()
    shutil.move(tmp, path)
    print("postprocess: cantSplit %d -> %d, pct %d, before180 %d, keepNext %d"
          % (n0, doc.count("<w:cantSplit/>"), doc.count('w:type="pct"'),
             doc.count('w:before="180"'), doc.count("<w:keepNext/>")))


if __name__ == "__main__":
    run(sys.argv[1])
