# -*- coding: utf-8 -*-
"""生成《LLC 软启动验证计划与记录台账》Excel"""
import os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "LLC软启动验证计划与记录台账.xlsx")

NAVY = "14375E"
LIGHT = "EAF1F8"
GREY = "F2F5F9"
thin = Side(style="thin", color="C8D0DA")
BORDER = Border(left=thin, right=thin, top=thin, bottom=thin)

wb = Workbook()


def style_header(ws, ncol, row=1):
    for c in range(1, ncol + 1):
        cell = ws.cell(row=row, column=c)
        cell.font = Font(name="微软雅黑", size=10, bold=True, color="FFFFFF")
        cell.fill = PatternFill("solid", fgColor=NAVY)
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.border = BORDER
    ws.row_dimensions[row].height = 30


def style_body(ws, nrow, ncol, start=2):
    for r in range(start, nrow + 1):
        for c in range(1, ncol + 1):
            cell = ws.cell(row=r, column=c)
            cell.font = Font(name="微软雅黑", size=9)
            cell.alignment = Alignment(vertical="top", wrap_text=True)
            cell.border = BORDER
            if r % 2 == 0:
                cell.fill = PatternFill("solid", fgColor=GREY)


# ============================================================
# Sheet 1 验证计划
# ============================================================
ws = wb.active
ws.title = "1-验证计划"

HEAD = ["编号", "层级", "验证项", "要回答的问题", "自变量", "观测量 / 测点",
        "预期结果", "通过判据", "工具"]
ws.append(HEAD)

PLAN = [
    # ---- L0 解析/脚本 ----
    ("V-01", "L0 脚本", "模型自检：FHA vs 时域",
     "我的模型可信吗？", "频率（5 点）", "稳态 Vout",
     "fn=1 误差 <1%，fn>1.3 误差 3~9%", "误差 <10% 才允许用模型下结论", "llc_verify.py 用例0"),
    ("V-02", "L0 脚本", "步长敏感性",
     "结论是不是数值噪声？", "每周期采样点数 120/320/640", "d=0.05 时稳态 Vout",
     "三者差异 <5%", "差异 >10% 则加密步长或放弃该结论", "llc_verify.py 用例0"),
    ("V-03", "L0 脚本", "增益曲线与量程下限",
     "纯 PFM 最低能到多少伏？", "频率", "M(f)、Vout_min",
     "Vout_min = M(fmax)·Vin/(2n)", "实测 Mmin 与设计值差 <10%", "脚本用例1"),
    ("V-04", "L0 脚本", "对称发波 d 扫描",
     "占空比到底能调多少？空档期中点去哪了？", "占空比 d（0.05~0.45）",
     "Vout、v_ab 基波、v_ab 均值", "增益随 d 单调上升后饱和",
     "找到饱和点；饱和点之后 d 无效", "脚本用例2 / 示波器"),
    ("V-05", "L0 脚本", "对称 d=0.45 vs 互补 50/50",
     "交接是不是真的零台阶？", "发波模式", "Vout、Cr 均值、i_Lr 峰值",
     "三者差异 <1%", "差异 <1% 则证明是同一状态", "脚本用例3"),
    ("V-06", "L0 脚本", "互补 30% 的 Cr 偏置",
     "小占空比会不会直流偏磁？", "占空比 0.30", "Cr 平均电压、i_Lr 正负峰",
     "Cr ≈ 0.3Vin；正负峰不等", "Cr 偏离 <5% 即伏秒平衡成立", "脚本用例4"),
    ("V-07", "L0 脚本", "交接增益台阶",
     "在哪个 d 交接最安全？", "交接点 d=0.05/0.10/0.20/0.30/0.45",
     "交接前后稳态 Vout", "台阶随 d 单调收敛", "台阶 <5% 视为安全", "脚本用例5"),
    ("V-08", "L0 脚本", "开机冲击",
     "软启动要压的是什么？", "无（直接 50/50 上电）",
     "首周期 i_Lr 峰值 / 稳态峰值", "倍数 >1，出现在前几个周期", "记录倍数与持续时间", "脚本用例6"),
    ("V-09", "L0 脚本", "ZVS 频率上限",
     "fmax 能抬多高？", "fmax、Lm、Coss", "fs_max = 1/sqrt(32·Lm·Coss)",
     "fmax < fs_max 且余量 >30%", "余量 <30% 则起扫段丢 ZVS", "脚本用例7"),
    ("V-10", "L0 脚本", "窄脉冲 vs 死区",
     "1% 占空比能不能发出来？", "d=1%、5%", "脉宽 vs 死区",
     "d=1%@200k → 50ns < 死区", "脉宽必须 > 死区 + 驱动延迟", "脚本用例8"),

    # ---- L1 电路仿真 ----
    ("S-01", "L1 仿真", "闭环软启动全过程",
     "整条轨迹有没有跳变？", "时间", "Vout、f 指令、i_Lr",
     "Vout 单调、f 单调下降、无过冲", "过冲 <5%，f 不触上限", "PLECS / PSIM"),
    ("S-02", "L1 仿真", "空载 / 满载 / 短路启动",
     "最恶劣工况会不会进容性区？", "负载", "f 指令、i_Lr 相位",
     "f 不低于 f_peak 钳位", "电流始终滞后电压", "PLECS"),
    ("S-03", "L1 仿真", "交接瞬间抓频率指令",
     "交接后 f 有没有顶到 fmax？", "交接点 d", "f 指令、Vout",
     "f 在 fmax 以下有调节余量", "f 触上限 = 量程不够，需改 tank", "PLECS"),
    ("S-04", "L1 仿真", "死区扫描",
     "死区多大才够 ZVS？", "死区 100~300ns", "Vds 开通前是否到 0",
     "存在一个最小死区", "Vds <10% Vin 视为 ZVS", "PLECS（需 Coss 模型）"),
    ("S-05", "L1 仿真", "器件损耗与结温",
     "软启动期间会不会过热？", "软启动时间 10/30/50ms", "单管损耗、ΔTj",
     "ΔTj < 20℃", "结温余量 >25℃", "PLECS Thermal"),

    # ---- L2 硬件 ----
    ("H-01", "L2 硬件", "Cr 电压稳态与纹波",
     "有没有直流偏置？纹波多大？", "占空比", "Cr 电压（差分探头）",
     "50/50 时 = Vin/2；d=0.3 时 = 0.3Vin", "均值偏离 <5%", "差分探头 + 示波器"),
    ("H-02", "L2 硬件", "原边电流正负对称",
     "有没有伏秒不平衡？", "占空比", "i_Lr 正负峰、基线漂移",
     "50/50 时正负对称", "不对称度 <5%", "电流互感器（带复位）/ 罗氏线圈"),
    ("H-03", "L2 硬件", "上下管壳温",
     "损耗是否不均？", "占空比 0.3 vs 0.5", "两管壳温",
     "d=0.3 时温差明显", "温差 >5℃ 即证实损耗不均", "热电偶 / 热像仪"),
    ("H-04", "L2 硬件", "中点电压实际波形",
     "空档期到底被填平没有？", "占空比 0.1/0.2/0.3/0.45", "v_ab 波形",
     "d 越大越接近方波", "d=0.45 应与 50/50 重合", "差分探头（高压）"),
    ("H-05", "L2 硬件", "栅极波形 vs 死区",
     "窄脉冲真的发得出来吗？", "d=1%/3%/5%", "Vgs 脉宽、Vds",
     "脉宽 <200ns 时波形畸变", "Vgs 平台完整 = 可用", "差分探头 ×2"),
    ("H-06", "L2 硬件", "开机冲击电流",
     "冲击倍数多少？持续多久？", "软启动 vs 硬启动", "i_Lr 峰值、持续时间",
     "硬启动倍数 = 仿真的 3~4 倍", "倍数 <2 视为软启动有效", "电流探头 + 单次触发"),
    ("H-07", "L2 硬件", "交接瞬间 Vout 与 f 指令",
     "振荡是不是交接引起的？", "交接点 d", "Vout、f 指令、i_Lr",
     "无台阶、f 不触上限", "若 f 触上限则改 tank 不改发波", "示波器 3 通道"),
    ("H-08", "L2 硬件", "效率与温升扫频",
     "全频段有没有异常点？", "频率 60k~200k", "效率、壳温",
     "效率曲线单峰、无凹陷", "凹陷 = 进入容性区", "功率分析仪"),
]

for row in PLAN:
    ws.append(list(row))

style_header(ws, len(HEAD))
style_body(ws, ws.max_row, len(HEAD))
widths = [7, 9, 22, 26, 20, 24, 26, 24, 20]
for i, w in enumerate(widths, 1):
    ws.column_dimensions[get_column_letter(i)].width = w
ws.freeze_panes = "C2"
for r in range(2, ws.max_row + 1):
    ws.row_dimensions[r].height = 46


# ============================================================
# Sheet 2 测试记录
# ============================================================
ws2 = wb.create_sheet("2-测试记录")
H2 = ["日期", "编号", "验证项", "参数基线（Vin/n/Lr/Cr/Lm/fmax）", "自变量取值",
      "观测量", "实测值", "预期值", "偏差", "结论（通过/存疑/推翻）",
      "截图文件名", "备注 / 下一步"]
ws2.append(H2)

# 三条示例，示范怎么写
EX = [
    ("2026-11-04", "V-06", "互补 30% 的 Cr 偏置",
     "400V/n16/Lr60µ/Cr42n/Lm78µ/200k", "d=0.30, f=200k",
     "Cr 平均电压", "118.0 V", "120.0 V", "-1.7%", "通过",
     "fig_cr_bias.png", "说明稳态伏秒自动平衡，偏磁说法不成立"),
    ("2026-11-04", "V-05", "对称 0.45 vs 互补 50/50",
     "同上", "d=0.45 / d=0.50",
     "Vout、Cr、i_Lr 峰值", "6.221 / 6.238 V", "差异 <1%", "0.28%", "通过",
     "fig_equiv.png", "证明是同一条曲线的两端"),
    ("", "H-04", "中点电压实际波形（待做）",
     "以实物为准", "d=0.1/0.2/0.3/0.45",
     "v_ab 波形", "", "d 越大越接近方波", "", "",
     "", "重点看 d=0.45 是否与 50/50 重合"),
]
for r in EX:
    ws2.append(list(r))
for _ in range(40):
    ws2.append([""] * len(H2))

style_header(ws2, len(H2))
style_body(ws2, ws2.max_row, len(H2))
for i, w in enumerate([11, 7, 22, 30, 18, 20, 20, 20, 9, 14, 20, 30], 1):
    ws2.column_dimensions[get_column_letter(i)].width = w
ws2.freeze_panes = "C2"


# ============================================================
# Sheet 3 参数基线
# ============================================================
ws3 = wb.create_sheet("3-参数基线")
H3 = ["参数", "符号", "数值", "单位", "来源（数据手册/实测/设计值）", "备注"]
ws3.append(H3)
PAR = [
    ("母线电压", "Vin", 400, "V", "设计值", "最低/最高都要记"),
    ("匝比", "n = Np:Ns", 16, "", "设计值", "决定 Vo_res = Vin/(2n)"),
    ("谐振频率", "fr", 100, "kHz", "设计值", "1/(2π√(LrCr))"),
    ("谐振电感", "Lr", 60, "µH", "实测", "含变压器漏感"),
    ("谐振电容", "Cr", 42, "nF", "实测", "注意直流偏压下的容值衰减"),
    ("励磁电感", "Lm", 78, "µH", "实测", "Ln = Lm/Lr"),
    ("单管输出电容", "Coss", 1.5, "nF", "数据手册", "取 Vds=400V 时的值"),
    ("死区", "td", 150, "ns", "设计值", "含驱动传输延迟差"),
    ("最高频率", "fmax", 200, "kHz", "设计值", "对应 Vout_min"),
    ("最低频率", "fmin", 60, "kHz", "设计值", "必须 > f_peak"),
    ("输出电容", "Cout", 1000, "µF", "设计值", ""),
    ("额定负载", "Rload", 0.36, "Ω", "设计值", "24V/1600W"),
]
for r in PAR:
    ws3.append(list(r))
style_header(ws3, len(H3))
style_body(ws3, ws3.max_row, len(H3))
for i, w in enumerate([18, 12, 10, 8, 26, 34], 1):
    ws3.column_dimensions[get_column_letter(i)].width = w


# ============================================================
# Sheet 4 结论台账
# ============================================================
ws4 = wb.create_sheet("4-结论台账")
H4 = ["编号", "原始说法", "证据（用例/图）", "判定", "为什么", "对设计的影响"]
ws4.append(H4)
CONC = [
    ("C-01", "小占空比会直流偏磁、饱和烧管", "V-06",
     "推翻", "Cr 本身就是隔直电容，稳态自动稳到 d·Vin；磁通摆幅 ∝ d，d 小反而离饱和更远",
     "不要用「防偏磁」当理由去限制 d；真正要防的是电流不对称带来的损耗不均"),
    ("C-02", "对称发波基波 ∝ sin²(πd)", "V-04",
     "推翻", "空档期中点被 Coss 拖着漂移，v_ab 是梯形不是窄脉冲串",
     "有效调节区间只有 d=0~0.2；d>0.3 加占空比无效"),
    ("C-03", "30% 切 50% 会因增益台阶振荡", "V-07",
     "推翻", "d=0.30 交接台阶仅 0.7%",
     "若实测仍振荡，去查频率环路（fmax 余量、积分器初值、限幅），不要改发波"),
    ("C-04", "对称发波和互补发波是两个模式", "V-05",
     "推翻", "d=0.45 与 50/50 的波形、Cr、电流差 <0.3%",
     "不需要「切换」，只需要把 d 爬到底；软件上可以省掉状态判断"),
    ("C-05", "1% 占空比可以做软启动", "V-10",
     "推翻", "200k 下 50ns < 死区 150ns，上管根本不开通",
     "d 下限取 5%，且必须用示波器确认栅极波形"),
    ("C-06", "危险在稳态磁通", "V-08",
     "修正", "首周期电流冲击是稳态的 3.4 倍，前 20 个周期内衰减",
     "软启动要压的是 Cr 充电电流，不是磁通"),
    ("C-07", "抬 fmax 可以无限扩增益范围", "V-09",
     "修正", "fs_max = 1/√(32·Lm·Coss) 是硬上限",
     "扩量程要么加 Lm，要么改匝比，别只抬 fmax"),
    ("C-08", "软启动要慢是为了散热", "V-08",
     "修正", "10ms 只有 0.16J，热根本不是约束",
     "慢是为了限制 Cr 充电电流和 EMI，10~50ms 足够"),
]
for r in CONC:
    ws4.append(list(r))
style_header(ws4, len(H4))
style_body(ws4, ws4.max_row, len(H4))
for i, w in enumerate([7, 34, 16, 9, 52, 48], 1):
    ws4.column_dimensions[get_column_letter(i)].width = w
for r in range(2, ws4.max_row + 1):
    ws4.row_dimensions[r].height = 58

wb.save(OUT)
print("saved:", OUT)
