# -*- coding: utf-8 -*-
"""把 KGNCommMonitor 的竖排文本日志（GBK，每记录 31 行 "时间  参数:值"）转成宽表 xlsx。"""
import io
import os
import re
import sys
from datetime import datetime

from openpyxl import Workbook
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.utils import get_column_letter

SRC = r"D:\Work\Anker\Anker\uppercomputer\A1727\KGNCommMonitor\KGNCommMonitor\bin\Debug\net6.0-windows\data\data_11_5_11_27_43.txt"

# 原文件中的参数顺序（每记录固定 31 行）
TEMPLATE = [
    "电池温度", "电池电压", "SOC", "电池温度区间", "最大电流",
    "140W最大功率", "140W分配功率", "140W运行功率", "140W连接状态", "140W_MOS温度", "140W过温标志",
    "MPPT最大功率", "MPPT分配功率", "MPPT运行功率", "MPPT连接状态", "MPPT_MOS温度", "MPPT过温标志",
    "充电模式", "CV状态", "PD输出功率", "MPPT输出功率", "PD分配状态", "MPPT分配状态",
    "PD100发送次数", "PD100接收次数", "PD140发送次数", "PD140接收次数",
    "MPPT成功次数", "MPPT失败次数", "BMS成功次数", "BMS失败次数",
]
# 附在某一参数行尾部、用双空格分隔的附加比率列
RATE_AFTER = {"PD100接收次数": "PD100帧错误率", "PD140接收次数": "PD140帧错误率",
              "MPPT失败次数": "MPPT失败率", "BMS失败次数": "BMS失败率"}

RATE_COLS = set(RATE_AFTER.values())

NUM_INT = {"SOC", "最大电流", "140W最大功率", "140W分配功率", "140W运行功率", "140W连接状态",
           "140W过温标志", "MPPT最大功率", "MPPT分配功率", "MPPT运行功率", "MPPT连接状态",
           "MPPT过温标志", "CV状态", "PD输出功率", "MPPT输出功率", "PD分配状态", "MPPT分配状态",
           "PD100发送次数", "PD100接收次数", "PD140发送次数", "PD140接收次数",
           "MPPT成功次数", "MPPT失败次数", "BMS成功次数", "BMS失败次数"}
NUM_1DP = {"电池温度", "140W_MOS温度", "MPPT_MOS温度"}
NUM_3DP = {"电池电压"}


def parse(path):
    with io.open(path, encoding="gbk", errors="replace") as f:
        raw = f.read().splitlines()
    lines = [l for l in raw if l.strip()]
    total = len(lines)
    if total % len(TEMPLATE):
        sys.exit("行数 %d 不是 %d 的整数倍，结构异常" % (total, len(TEMPLATE)))
    records = []
    for start in range(0, total, len(TEMPLATE)):
        block = lines[start:start + len(TEMPLATE)]
        rec = {}
        ts = None
        for idx, line in enumerate(block):
            m = re.match(r"^(\d{4}/\d{1,2}/\d{1,2}\s+\d{1,2}:\d{2}:\d{2})\s+(.*)$", line)
            if not m:
                sys.exit("第 %d 行无法解析时间戳: %r" % (start + idx + 1, line))
            cur_ts, body = m.group(1), m.group(2).strip()
            if ts is None:
                ts = cur_ts
            chunks = re.split(r"\s{2,}", body)
            label, _, val = chunks[0].partition(":")
            label = label.strip()
            if label != TEMPLATE[idx]:
                sys.exit("第 %d 行字段顺序异常: 期望 %s 实际 %s" % (start + idx + 1, TEMPLATE[idx], label))
            rec[label] = val.strip()
            if len(chunks) > 1 and label in RATE_AFTER:
                rl, _, rv = chunks[1].partition(":")
                rec[RATE_AFTER[label]] = rv.strip()
        records.append((ts, rec))
    return records


def to_num(text, kind):
    if text is None or text == "":
        return None
    t = text.replace("%", "").replace("℃", "").strip()
    try:
        v = float(t)
    except ValueError:
        return text
    if kind == "int":
        return int(v) if v == int(v) else v
    return v


def build_columns():
    cols = ["时间"]
    for name in TEMPLATE:
        cols.append(name)
        if name in RATE_AFTER:
            cols.append(RATE_AFTER[name])
    return cols


def main():
    records = parse(SRC)
    cols = build_columns()

    wb = Workbook()
    ws = wb.active
    ws.title = "数据"

    head_fill = PatternFill("solid", fgColor="1F4E79")
    head_font = Font(name="微软雅黑", size=10, bold=True, color="FFFFFF")
    body_font = Font(name="微软雅黑", size=10)
    thin = Side(style="thin", color="D9D9D9")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    ws.append(cols)
    for c in range(1, len(cols) + 1):
        cell = ws.cell(row=1, column=c)
        cell.fill = head_fill
        cell.font = head_font
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.border = border
    ws.row_dimensions[1].height = 30

    fmts = {}
    for i, name in enumerate(cols, start=1):
        if name == "时间":
            fmts[i] = "yyyy/m/d h:mm:ss"
        elif name in RATE_COLS:
            fmts[i] = "0.00%"
        elif name in NUM_INT:
            fmts[i] = "0"
        elif name in NUM_1DP:
            fmts[i] = "0.0"
        elif name in NUM_3DP:
            fmts[i] = "0.000"
        else:
            fmts[i] = "@"

    for ts, rec in records:
        row = [datetime.strptime(ts, "%Y/%m/%d %H:%M:%S")]
        for name in cols[1:]:
            v = rec.get(name)
            if name in RATE_COLS:
                if v is None:
                    row.append(None)
                else:
                    try:
                        row.append(float(v.replace("%", "")) / 100.0)
                    except ValueError:
                        row.append(v)
            elif name in NUM_INT:
                row.append(to_num(v, "int"))
            elif name in NUM_1DP or name in NUM_3DP:
                row.append(to_num(v, "num"))
            else:
                row.append(v)
        ws.append(row)

    nrows = ws.max_row
    for r in range(2, nrows + 1):
        for c in range(1, len(cols) + 1):
            cell = ws.cell(row=r, column=c)
            cell.font = body_font
            cell.number_format = fmts[c]
            cell.alignment = Alignment(horizontal="center", vertical="center")
        ws.cell(row=r, column=1).alignment = Alignment(horizontal="center", vertical="center")

    widths = {"时间": 19, "电池温度区间": 15, "充电模式": 10}
    for i, name in enumerate(cols, start=1):
        ws.column_dimensions[get_column_letter(i)].width = widths.get(name, max(10, len(name) * 1.7))

    ws.freeze_panes = "B2"
    ws.auto_filter.ref = "A1:%s%d" % (get_column_letter(len(cols)), nrows)

    # 概览页
    ov = wb.create_sheet("数据概览", 0)
    first = datetime.strptime(records[0][0], "%Y/%m/%d %H:%M:%S")
    last = datetime.strptime(records[-1][0], "%Y/%m/%d %H:%M:%S")
    info = [
        ("源文件", os.path.basename(SRC)),
        ("采样记录数", len(records)),
        ("参数列数", len(cols) - 1),
        ("起始时间", first),
        ("结束时间", last),
        ("持续时长", str(last - first)),
    ]
    ov["A1"] = "KGN 通信监控数据概览"
    ov["A1"].font = Font(name="微软雅黑", size=14, bold=True, color="1F4E79")
    ov.merge_cells("A1:B1")
    for i, (k, v) in enumerate(info, start=3):
        ov.cell(row=i, column=1, value=k).font = Font(name="微软雅黑", size=10, bold=True)
        c = ov.cell(row=i, column=2, value=v)
        c.font = Font(name="微软雅黑", size=10)
        if isinstance(v, datetime):
            c.number_format = "yyyy/m/d h:mm:ss"
        ov.cell(row=i, column=1).fill = PatternFill("solid", fgColor="DDEBF7")
    ov.column_dimensions["A"].width = 16
    ov.column_dimensions["B"].width = 30

    out = os.path.splitext(SRC)[0] + ".xlsx"
    wb.save(out)
    print("OK ->", out)
    print("records:", len(records), "cols:", len(cols), "rows:", nrows)


if __name__ == "__main__":
    main()
