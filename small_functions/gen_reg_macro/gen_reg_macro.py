import re
import os
import shutil
import sys
import tempfile
from pathlib import Path
from typing import Hashable

import pandas as pd


class GenRegMacro:
    # Define column indices
    address_col = 0
    sub_addr_col = 1
    start_bit_col = 2
    end_bit_col = 3
    bit_width_col = 4
    register_name_col = 7

    def __init__(self,file_name):
        current_dir = Path.cwd()
        self.file_path = os.path.join(current_dir, file_name)
        self.sheet_name = ["reg10"]#["reg10", "reg11", "reg0E", "reg0C", "reg0F", "reg0D"]


    def extract_content(self):
        # Load the Excel file
        for sheet_name in self.sheet_name:
            # print(sheet_name)
            excel_data = pd.read_excel(self.file_path, sheet_name=sheet_name, dtype=str)
            for index, row in excel_data.iterrows():
                # print(row[self.sub_addr_col])
                if not pd.isna(row.iloc[self.address_col]):
                    print(excel_data.iloc[index + 1, self.start_bit_col])
                    address = row.iloc[self.address_col]
                    sub_addr= row.iloc[self.sub_addr_col]
                    new_32bit_addr = "0x"+address+sub_addr
                    print(new_32bit_addr)
                    # print(execl_data.iterrows[index+1].iloc[self.start_bit_col])











def main(input_file):
    gen_reg_macro = GenRegMacro(input_file)
    gen_reg_macro.extract_content()
    #gen_reg_macro.gen_output_file()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 gen_reg_macro.py <excel input_file>")
        sys.exit(1)
    input_file = sys.argv[1]
    main(input_file)
