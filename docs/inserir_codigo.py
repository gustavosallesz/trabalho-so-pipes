import zipfile, shutil, os, re
from xml.sax.saxutils import escape

DOCX = "update-relatorio_adivinhacao.docx"
CODIGO = "../src/adivinhacao.c"
SAIDA = "update-relatorio_adivinhacao.docx"

with open(CODIGO, "r", encoding="utf-8") as f:
    linhas = f.readlines()

def linha_para_xml(linha):
    texto = escape(linha.rstrip("\n"))
    return (
        '<w:p>'
        '<w:pPr><w:jc w:val="left"/></w:pPr>'
        '<w:r>'
        '<w:rPr>'
        '<w:rFonts w:ascii="Courier New" w:hAnsi="Courier New"/>'
        '<w:sz w:val="18"/>'
        '</w:rPr>'
        f'<w:t xml:space="preserve">{texto}</w:t>'
        '</w:r>'
        '</w:p>'
    )

titulo_xml = (
    '<w:p>'
    '<w:pPr><w:jc w:val="left"/></w:pPr>'
    '<w:r>'
    '<w:rPr>'
    '<w:b/>'
    '<w:sz w:val="24"/>'
    '</w:rPr>'
    '<w:t>Codigo Fonte: adivinhacao.c</w:t>'
    '</w:r>'
    '</w:p>'
)

paragrafos_codigo = titulo_xml + "".join(linha_para_xml(l) for l in linhas)

tmp = DOCX + ".tmp"
shutil.copy(DOCX, tmp)

with zipfile.ZipFile(tmp, "r") as zin:
    nomes = zin.namelist()
    conteudos = {nome: zin.read(nome) for nome in nomes}

xml = conteudos["word/document.xml"].decode("utf-8")
xml = xml.replace("</w:body>", paragrafos_codigo + "</w:body>")
conteudos["word/document.xml"] = xml.encode("utf-8")

with zipfile.ZipFile(SAIDA, "w", zipfile.ZIP_DEFLATED) as zout:
    for nome, dados in conteudos.items():
        zout.writestr(nome, dados)

os.remove(tmp)
print("Pronto! Codigo inserido em:", SAIDA)
