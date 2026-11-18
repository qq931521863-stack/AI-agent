#!/usr/bin/env python3
"""Append APP_A / APP_B Keil targets from old IAP project into R02 uvprojx."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
R02_PROJ = ROOT / "xpPFC431_48.uvprojx"
OLD_PROJ = ROOT.parent.parent / "xpPFC1600_v1.0.3_release_6.6" / "MDK-ARM" / "xpPFC431_48.uvprojx"

UPGRADE_INCLUDE = (
    "../Core/Inc;../Drivers/STM32G4xx_HAL_Driver/Inc;"
    "../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy;"
    "../Drivers/CMSIS/Device/ST/STM32G4xx/Include;../Drivers/CMSIS/Include;"
    "../AppUser;../pfc;../Math_Lib/inc;../User;../User/UartUpgrade;"
    "../Driver/USART;../Service/pfc_safe;../Service/Flash_Internal;../Service/CRC"
)

UPGRADE_GROUPS = """
        <Group>
          <GroupName>User</GroupName>
          <Files>
            <File><FileName>upgrade_app.c</FileName><FileType>1</FileType><FilePath>..\\User\\upgrade_app.c</FilePath></File>
            <File><FileName>UpgradeDebugLog.c</FileName><FileType>1</FileType><FilePath>..\\User\\UpgradeDebugLog.c</FilePath></File>
            <File><FileName>Timer_pro.c</FileName><FileType>1</FileType><FilePath>..\\User\\Timer_pro.c</FilePath></File>
            <File><FileName>Uart_Rec_Analysis.c</FileName><FileType>1</FileType><FilePath>..\\User\\Uart_Rec_Analysis.c</FilePath></File>
            <File><FileName>UartFrameHelper.c</FileName><FileType>1</FileType><FilePath>..\\User\\UartFrameHelper.c</FilePath></File>
            <File><FileName>InquireDevMsg.c</FileName><FileType>1</FileType><FilePath>..\\User\\InquireDevMsg.c</FilePath></File>
            <File><FileName>UpgradeMsgSaveFunctionDf.c</FileName><FileType>1</FileType><FilePath>..\\User\\UpgradeMsgSaveFunctionDf.c</FilePath></File>
            <File><FileName>AppSlotValidate.c</FileName><FileType>1</FileType><FilePath>..\\User\\AppSlotValidate.c</FilePath></File>
            <File><FileName>UartPort.c</FileName><FileType>1</FileType><FilePath>..\\User\\UartPort.c</FilePath></File>
            <File><FileName>UartRecUpgradeCmdRetuenDf.c</FileName><FileType>1</FileType><FilePath>..\\User\\UartUpgrade\\UartRecUpgradeCmdRetuenDf.c</FilePath></File>
            <File><FileName>usart3_debug.c</FileName><FileType>1</FileType><FilePath>..\\Driver\\USART\\usart3_debug.c</FilePath></File>
          </Files>
        </Group>
        <Group>
          <GroupName>Service</GroupName>
          <Files>
            <File><FileName>PfcSafeHw.c</FileName><FileType>1</FileType><FilePath>..\\Service\\pfc_safe\\PfcSafeHw.c</FilePath></File>
            <File><FileName>PfcSafeAppBridge.c</FileName><FileType>1</FileType><FilePath>..\\Service\\pfc_safe\\PfcSafeAppBridge.c</FilePath></File>
            <File><FileName>flash_internal.c</FileName><FileType>1</FileType><FilePath>..\\Service\\Flash_Internal\\flash_internal.c</FilePath></File>
            <File><FileName>crc16.c</FileName><FileType>1</FileType><FilePath>..\\Service\\CRC\\crc16.c</FilePath></File>
          </Files>
        </Group>"""


def extract_target(text: str, name: str) -> str:
    pattern = rf"(<Target>\s*<TargetName>{re.escape(name)}</TargetName>.*?</Target>)"
    m = re.search(pattern, text, re.DOTALL)
    if not m:
        raise SystemExit(f"Target {name} not found")
    return m.group(1)


def patch_target(block: str, *, slot: str, irom_start: str, irom_size: str, scatter: str, output: str) -> str:
    block = block.replace("<TargetName>xpPFC431_48_A</TargetName>", f"<TargetName>APP_{slot}</TargetName>")
    block = block.replace("<TargetName>xpPFC431_48_B</TargetName>", f"<TargetName>APP_{slot}</TargetName>")
    block = re.sub(
        r"<Cpu>IRAM\(0x20000000-0x20007FFF\) IROM\([^)]+\)[^<]*</Cpu>",
        f"<Cpu>IRAM(0x20000000-0x20007FFF) IROM({irom_start}-{hex(int(irom_start, 16) + int(irom_size, 16) - 1).replace('0x', '0x8') if irom_start.startswith('0x80') else 'bad'})  CLOCK(8000000) FPU2 CPUTYPE(\"Cortex-M4\") TZ</Cpu>",
        block,
        count=1,
    )
    # simpler Cpu line replacement
    if slot == "A":
        block = re.sub(
            r"<Cpu>[^<]+</Cpu>",
            "<Cpu>IRAM(0x20000000-0x20007FFF) IROM(0x8008000-0x8012FFF)  CLOCK(8000000) FPU2 CPUTYPE(\"Cortex-M4\") TZ</Cpu>",
            block,
            count=1,
        )
    else:
        block = re.sub(
            r"<Cpu>[^<]+</Cpu>",
            "<Cpu>IRAM(0x20000000-0x20007FFF) IROM(0x8013000-0x801DFFF)  CLOCK(8000000) FPU2 CPUTYPE(\"Cortex-M4\") TZ</Cpu>",
            block,
            count=1,
        )

    block = block.replace("<OutputName>APP_A</OutputName>", f"<OutputName>{output}</OutputName>")
    block = block.replace("<OutputName>APP_B</OutputName>", f"<OutputName>{output}</OutputName>")

    block = re.sub(
        r"<Define>USE_HAL_DRIVER,STM32G431xx,APP_SLOT_[AB]_BUILD</Define>",
        f"<Define>USE_HAL_DRIVER,STM32G431xx,APP_SLOT_{slot}_BUILD</Define>",
        block,
        count=1,
    )
    block = re.sub(
        r"<IncludePath>[^<]+</IncludePath>",
        f"<IncludePath>{UPGRADE_INCLUDE}</IncludePath>",
        block,
        count=1,
    )
    block = re.sub(
        r"<StartAddress>0x8008000</StartAddress>\s*<Size>0xb000</Size>",
        f"<StartAddress>{irom_start}</StartAddress>\n                <Size>{irom_size}</Size>",
        block,
    )
    block = block.replace(
        "<ScatterFile>.\\xpPFC431_48\\APP_A.sct</ScatterFile>",
        f"<ScatterFile>.\\xpPFC431_48\\{scatter}</ScatterFile>",
    )
    block = block.replace(
        "<ScatterFile>.\\xpPFC431_48\\APP_B.sct</ScatterFile>",
        f"<ScatterFile>.\\xpPFC431_48\\{scatter}</ScatterFile>",
    )
    block = re.sub(
        r"<useFile>0</useFile>",
        "<useFile>1</useFile>",
        block,
        count=1,
    )
    if "<GroupName>User</GroupName>" not in block:
        block = block.replace(
            '          <GroupName>::CMSIS</GroupName>\n        </Group>\n      </Groups>',
            UPGRADE_GROUPS + '\n        <Group>\n          <GroupName>::CMSIS</GroupName>\n        </Group>\n      </Groups>',
        )
    # PfcAlgorithm group name in R02
    block = block.replace("<GroupName>Math_Lib</GroupName>", "<GroupName>PfcAlgorithm</GroupName>")
    block = block.replace(
        '<FilePath>..\\Math_Lib\\lib\\math_pfc.lib</FilePath>',
        '<FilePath>..\\Math_Lib\\lib\\math_pfc.lib</FilePath>',
    )
    return block


def main() -> None:
    r02 = R02_PROJ.read_text(encoding="utf-8")
    old = OLD_PROJ.read_text(encoding="utf-8")

    if "<TargetName>APP_A</TargetName>" in r02:
        print("APP_A/APP_B already present, skip")
        return

    tgt_a = patch_target(
        extract_target(old, "xpPFC431_48_A"),
        slot="A",
        irom_start="0x8008000",
        irom_size="0xb000",
        scatter="APP_A.sct",
        output="APP_A",
    )
    tgt_b = patch_target(
        extract_target(old, "xpPFC431_48_B"),
        slot="B",
        irom_start="0x8013000",
        irom_size="0xb000",
        scatter="APP_B.sct",
        output="APP_B",
    )

    # Fix PfcAlgorithm in copied targets (old used AppUser group only)
    for tgt in (tgt_a, tgt_b):
        if "<GroupName>PfcAlgorithm</GroupName>" not in tgt:
            tgt.replace(
                "</Group>\n        <Group>\n          <GroupName>User</GroupName>",
                "</Group>\n        <Group>\n          <GroupName>PfcAlgorithm</GroupName>\n          <Files>\n            <File><FileName>math_pfc.lib</FileName><FileType>4</FileType><FilePath>..\\Math_Lib\\lib\\math_pfc.lib</FilePath></File>\n          </Files>\n        </Group>\n        <Group>\n          <GroupName>User</GroupName>",
            )

    r02 = r02.replace("  </Targets>", f"    {tgt_a}\n    {tgt_b}\n  </Targets>")

    r02 = r02.replace(
        '<targetInfo name="xpPFC431_48"/>',
        '<targetInfo name="xpPFC431_48"/>\n          <targetInfo name="APP_A"/>\n          <targetInfo name="APP_B"/>',
    )

    R02_PROJ.write_text(r02, encoding="utf-8")
    print("Merged APP_A and APP_B targets into", R02_PROJ)


if __name__ == "__main__":
    main()
