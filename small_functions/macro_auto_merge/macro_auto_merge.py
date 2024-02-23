import re
import os
import shutil
import sys
import tempfile
from pathlib import Path



class MacroAutoMerge:
    def __init__(self, old_relative_path, new_relative_path, endname):
        current_dir = Path.cwd()
        self.old_path = os.path.join(current_dir, old_relative_path)
        self.new_path = os.path.join(current_dir, new_relative_path)
        self.endname = endname
        self.old_file_dict = {}
        self.old_file_del_dict = {}

    def find_files(self, path):
        files = []
        for f in os.listdir(path):
            if f.endswith(self.endname):
                files.append(f)
        return files

    def load_conent(self, file_path):
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.readlines()
            f.close()
        return content

    def load_all_data(self):
        for file in self.find_files(self.old_path):
            self.old_file_dict.setdefault(file, {})
            self.old_file_del_dict.setdefault(file, {})
            file_path = os.path.join(self.old_path, file)
            # print(file_path)
            self.extract_data(file_name=file, file_path_name=file_path)

    def extract_data(self, file_name, file_path_name):
        # get_file_content by func

        ifdef_stack = []
        fpga_index = 0
        fpga_content_flag = False
        del_content_flag = False
        file_content = self.load_conent(file_path_name)
        self.match_cfg()

        for index, line in enumerate(file_content):
            get_ifdef = self.get_macro_ifdef.match(line)
            get_else = self.get_macro_else.match(line)
            if get_ifdef:
                ifdef_stack.append(get_ifdef.group(1))
                #print(ifdef_stack)
                if get_ifdef.group(1) == 'FPGA':
                    if fpga_content_flag:
                        print(f"Error occurred due to FPGA block not close")
                    else :
                        macro_block = []
                        macro_block.append(file_content[index - 2])
                        macro_block.append(file_content[index - 1])
                        fpga_content_flag = True

                        del_block = []

            if fpga_content_flag:
                get_endif = self.get_macro_endif.match(line)

                if get_endif:
                    current_ifdef = ifdef_stack[-1]
                    #print(current_ifdef)
                    if len(ifdef_stack) == 1:
                        fpga_content_flag = False
                        macro_block.append(line)
                        if del_content_flag:
                            del_block.append(line)
                            del_content_flag = False

                        self.old_file_dict.setdefault(file_name, {}).setdefault(fpga_index, macro_block)
                        self.old_file_del_dict.setdefault(file_name, {}).setdefault(fpga_index, del_block)
                        fpga_index = fpga_index + 1
                        ifdef_stack.pop()
                        #print(ifdef_stack)
                        #print(del_block)
                    else:
                        macro_block.append(line)
                        ifdef_stack.pop()
                else:
                    macro_block.append(line)

                if get_else:
                    if len(ifdef_stack) == 1:
                        del_content_flag = True

                if del_content_flag :
                    del_block.append(line)

        # print(macro_block)

    def insert_data(self):
        for file_name, content_dict in self.old_file_dict.items():
            #print(file_name)
            #print(content_dict)
            file_path = os.path.join(self.new_path, file_name)
            old_path = os.path.join(self.old_path, file_name)
            # print(file_path)
            if not content_dict :
                print(f"file {file_name} don't have FPGA macro")
            else :
                print(f"file {file_name} have FPGA macro")

                if os.path.exists(file_path):
                    # print(f"File {file_path} exists")
                    with open(file_path, 'r', encoding='utf-8') as f:
                        # print(f"File {file_path} ")
                        file_content = f.readlines()
                        with tempfile.NamedTemporaryFile(mode='w+', encoding='utf-8', delete=False) as tmp_file:
                            for block_index, line_list in content_dict.items():
                                for index, line in enumerate(file_content):
                                    if line == line_list[0]:
                                        if file_content[index + 1] == line_list[1]:
                                            for i, block_line in enumerate(line_list[2:]):
                                                file_content.insert(index + 2 + i, block_line)
                            # Delete duplicate rows, the macro must be written in format
                            for del_index, del_content_block in self.old_file_del_dict[file_name].items():
                                if not del_content_block:
                                    #print(f"file {file_name}:{del_index} don't have del block")
                                    pass
                                else:
                                    print(f"file {file_name}:{del_index} have del block")
                                    del_block_length = len(del_content_block)
                                    for index, line in enumerate(file_content):
                                        if line == del_content_block[0]:
                                            if file_content[index + 1] == del_content_block[1]:
                                                for i in range(del_block_length -2):
                                                    #print(f" need delete    {del_content_block[1+i]}")
                                                    #print(f" delete         {file_content[index + del_block_length]}")
                                                    if del_content_block[1+i] == file_content[index + del_block_length]:
                                                        del file_content[index + del_block_length]
                                                    else :
                                                        print(f" Error occurred due to {del_content_block[1+i]} not equal {file_content[index + del_block_length]}")
                                                        sys.exit(1)
                                                break


                            # print(file_content)
                            # 将文件指针重定位到文件的开始位置
                            tmp_file.seek(0)
                            # 将文件剪裁至当前指针位置
                            tmp_file.truncate()
                            tmp_file.writelines(file_content)
                            tmp_file.close()
                            shutil.move(tmp_file.name, old_path)
                            f.close()

                else:
                    print(f"File {file_path} NO exists")

                #src = file_path
                #dst = old_path
                #self.copy_file(src, dst)

    def match_cfg(self):
        self.get_macro_ifdef = re.compile("\s*`ifdef\s*(\w+)")
        self.get_macro_else = re.compile("\s*`else")
        self.get_macro_endif = re.compile("\s*`endif")

    def copy_file(self, src_path, dst_path):
        try:
            with open(src_path, 'rb') as src_file:
                with open(dst_path, 'wb') as dst_file:
                    data = src_file.read()
                    dst_file.write(data)
        except IOError as e:
            print(f"Error occurred while copying file: {e}")

    # 使用方法


def main():
    merge = MacroAutoMerge(old_relative_path='old',
                           new_relative_path='new',
                           endname='.v')
    merge.load_all_data()
    merge.insert_data()


if __name__ == '__main__':
    main()
