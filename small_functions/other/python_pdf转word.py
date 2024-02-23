# from pdf2docx import Converter
#
# pdf_file = '/path/to/sample.pdf'
# docx_file = 'path/to/sample.docx'
#
# # convert pdf to docx
# cv = Converter(pdf_file)
# cv.convert(docx_file) # 默认参数start=0, end=None
# cv.close()
#
# # more samples
# # cv.convert(docx_file, start=1) # 转换第2页到最后一页
# # cv.convert(docx_file, pages=[1,3,5]) # 转换第2，4，6页

import os
from pdf2docx import parse
from win32com import client

# 定义输入和输出文件路径
pdf_file = 'C:/FPGA/rfsoc/hzc_tmp/1_启明星ZYNQ之FPGA开发指南_V1.1.1.pdf'
docx_file = 'C:/FPGA/rfsoc/hzc_tmp/1_启明星ZYNQ之FPGA开发指南_V1.1.1.docx'

# 将 PDF 文件转换为 DOCX 文件
parse(pdf_file, docx_file)

# 打开 Microsoft Word 程序
word = client.Dispatch('Word.Application')

# 打开 DOCX 文件
doc = word.Documents.Open(docx_file)

# 保存 DOCX 文件
doc.Save()

# 关闭 DOCX 文件和 Word 程序
doc.Close()
word.Quit()