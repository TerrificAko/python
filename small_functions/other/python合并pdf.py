
import os
import PyPDF2

# 打开要合并的 PDF 文件
input_dir = 'C:/FPGA/rfsoc/hzc_tmp/'
input_files = ['Vivado使用教程.pdf', 'Vivado简明教程.pdf']

# 创建一个空的 PDF 文件对象
output_pdf = PyPDF2.PdfWriter()

# 逐个读取文件并将其添加到输出 PDF 文件对象中
for input_file in input_files:
    with open(os.path.join(input_dir, input_file), 'rb') as f:
        input_pdf = PyPDF2.PdfReader(f)
        #for i in range(input_pdf.getNumPages()):
        for i in range(len(input_pdf.pages)):
            # if input_file == 'B.pdf' and i == 1:
            #     #删除 B.pdf 的第二页
            #     continue
            # elif input_file == 'B.pdf' and i == 3:
            #     # 将 B.pdf 的第四页插入到 A.pdf 的第二页
            #     output_pdf.add_page(input_pdf.pages[3])
            # else:
                output_pdf.add_page(input_pdf.pages[i])


# 将输出 PDF 文件对象写入到新文件中
with open(os.path.join(input_dir, 'merged.pdf'), 'wb') as f:
    output_pdf.write(f)


# import os
# import PyPDF2
#
# # 打开要合并的 PDF 文件
# input_dir = 'C:/FPGA/rfsoc/hzc_tmp/'
# input_files = ['A.pdf', 'B.pdf']
#
# # 创建一个空的 PDF 文件对象
# output_pdf = PyPDF2.PdfFileWriter()
#
# # 逐个读取文件并将其添加到输出 PDF 文件对象中
# for input_file in input_files:
#     with open(os.path.join(input_dir, input_file), 'rb') as f:
#         input_pdf = PyPDF2.PdfFileReader(f)
#         for i in range(input_pdf.getNumPages()):
#             if input_file == 'B.pdf' and i == 1:
#                 # 删除 B.pdf 的第二页
#                 continue
#             elif input_file == 'B.pdf' and i == 3:
#                 # 将 B.pdf 的第四页插入到 A.pdf 的第二页
#                 output_pdf.addPage(input_pdf.getPage(3))
#             else:
#                 output_pdf.addPage(input_pdf.getPage(i))
#
# # 将输出 PDF 文件对象写入到新文件中
# with open(os.path.join(input_dir, 'merged.pdf'), 'wb') as f:
#     output_pdf.write(f)