import re
import os
from pathlib import Path


class MacroAutoMerge:
    def __init__(self, old_relative_path, new_relative_path, endname):
        # 使用os.getcwd()获取当前工作目录，并结合相对路径构造绝对路径
        current_dir = Path.cwd()
        self.old_path = os.path.join(current_dir, old_relative_path)
        self.new_path = os.path.join(current_dir, new_relative_path)
        self.endname = endname
        self.old_file_dict = {}
        self.new_file_dict = {}

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
            file_path = os.path.join(self.old_path, file)
            # print(file_path)
            self.extract_data(file_name=file, file_path_name=file_path)

    def extract_data(self, file_name, file_path_name):
        # get_file_content by func
        fpga_index = 0
        fpga_content_flag = False
        file_content = self.load_conent(file_path_name)
        self.match_cfg()

        for index, line in enumerate(file_content):
            get_fpga = self.get_macro_fpga.match(line)
            if get_fpga:
                macro_block = []
                fpga_content_flag = True
                macro_block.append(file_content[index - 2])
                macro_block.append(file_content[index - 1])

            if fpga_content_flag:
                get_endif = self.get_macro_endif.match(line)

                if get_endif:
                    fpga_content_flag = False
                    macro_block.append(line)
                    self.old_file_dict.setdefault(file_name, {}).setdefault(fpga_index, macro_block)
                    fpga_index = fpga_index + 1
                else:
                    macro_block.append(line)
        # print(macro_block)

    def insert_data(self):
        for file_name, content_dict in self.old_file_dict.items():
            # print(file_name)
            file_path = os.path.join(self.new_path, file_name)
            old_path = os.path.join(self.old_path, file_name)
            # print(file_path)
            if os.path.exists(file_path):
                # print(f"File {file_path} exists")
                with open(file_path, 'r+', encoding='utf-8') as f:
                    # print(f"File {file_path} ")
                    file_content = f.readlines()
                    for index, line in enumerate(file_content):
                        for block_index, line_list in content_dict.items():
                            match_block = re.compile(line_list[0])
                            if match_block.match(line):
                                if file_content[index + 1] == line_list[1]:
                                    for i, block_line in enumerate(line_list[2:]):
                                        file_content.insert(index + 2 + i, block_line)

                                        # print(line)
                    # print(file_content)
                    f.seek(0)
                    f.truncate()
                    f.writelines(file_content)
                    f.close()
            else:
                print(f"File {file_path} NO exists")

            src = file_path
            dst = old_path
            self.copy_file(src, dst)

    def match_cfg(self):
        self.get_macro_fpga = re.compile("`ifdef FPGA")
        self.get_macro_endif = re.compile("`endif")

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
