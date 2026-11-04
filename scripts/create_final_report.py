#!/usr/bin/env python3
"""Build the editable final Word report from docs/FINAL_REPORT.md."""
import re
from pathlib import Path

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "docs" / "FINAL_REPORT.md"
OUTPUT = ROOT / "docs" / "SOC_Alert_Correlation_Final_Report.docx"
TOKEN = re.compile(r"(\*\*[^*]+\*\*|\[[^]]+\]\([^)]+\))")


def rich(paragraph, text):
    for part in TOKEN.split(text):
        if not part:
            continue
        if part.startswith("**"):
            paragraph.add_run(part[2:-2]).bold = True
        elif part.startswith("["):
            label, target = part[1:].split("](", 1)
            target = target[:-1]
            paragraph.add_run(label)
            if target.startswith("http"):
                paragraph.add_run(" (" + target + ")").italic = True
        else:
            paragraph.add_run(part.replace("`", ""))


def main():
    doc = Document()
    section = doc.sections[0]
    section.page_width, section.page_height = Cm(21), Cm(29.7)
    section.top_margin = section.bottom_margin = Cm(2.1)
    section.left_margin = section.right_margin = Cm(2.25)

    normal = doc.styles["Normal"]
    normal.font.name = "Aptos"
    normal.font.size = Pt(10.5)
    normal.font.color.rgb = RGBColor(28, 39, 44)
    normal.paragraph_format.space_after = Pt(8)
    normal.paragraph_format.line_spacing = 1.15
    normal.paragraph_format.keep_together = True
    for name, size, before, after in [("Title", 22, 0, 11), ("Heading 1", 14, 17, 7)]:
        style = doc.styles[name]
        style.font.name = "Aptos Display"
        style.font.size = Pt(size)
        style.font.bold = name != "Title"
        style.font.color.rgb = RGBColor(23, 49, 59)
        style.paragraph_format.space_before = Pt(before)
        style.paragraph_format.space_after = Pt(after)
        style.paragraph_format.keep_with_next = True
    doc.styles["Title"].paragraph_format.space_after = Pt(5)
    doc.styles["Title"].font.color.rgb = RGBColor(0, 0, 0)
    title_properties = doc.styles["Title"]._element.pPr
    if title_properties is not None:
        for border in title_properties.findall(qn("w:pBdr")):
            title_properties.remove(border)
    for style_name in ("List Bullet", "List Number"):
        style = doc.styles[style_name]
        style.font.name = "Aptos"
        style.font.size = Pt(10.5)
        style.paragraph_format.space_after = Pt(5)

    lines = SOURCE.read_text(encoding="utf-8").splitlines()
    in_code = False
    code_lines = []
    for line in lines:
        if line.startswith("```"):
            if in_code:
                p = doc.add_paragraph()
                p.style = "Normal"
                p.paragraph_format.left_indent = Cm(0.45)
                p.paragraph_format.space_before = Pt(3)
                p.paragraph_format.space_after = Pt(10)
                run = p.add_run("\n".join(code_lines))
                run.font.name = "Aptos Mono"
                run.font.size = Pt(8.5)
                code_lines = []
            in_code = not in_code
            continue
        if in_code:
            code_lines.append(line)
            continue
        if not line.strip():
            continue
        if line.startswith("# "):
            doc.add_paragraph(line[2:], "Title")
        elif line.startswith("## "):
            doc.add_paragraph(line[3:], "Heading 1")
        elif re.match(r"^\d+\. ", line):
            p = doc.add_paragraph(style="List Number")
            rich(p, re.sub(r"^\d+\. ", "", line))
        elif line.startswith("- "):
            p = doc.add_paragraph(style="List Bullet")
            rich(p, line[2:])
        else:
            p = doc.add_paragraph()
            rich(p, line)
            if line.startswith("**Final project report"):
                p.paragraph_format.space_after = Pt(14)
                for run in p.runs:
                    run.font.color.rgb = RGBColor(83, 99, 104)
                    run.font.size = Pt(9)

    footer = section.footer.paragraphs[0]
    footer.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    footer.add_run("SOC Alert Correlation  •  ").font.size = Pt(8)
    field = OxmlElement("w:fldSimple")
    field.set(qn("w:instr"), "PAGE")
    footer._p.append(field)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    doc.save(OUTPUT)
    print(OUTPUT)


if __name__ == "__main__":
    main()
