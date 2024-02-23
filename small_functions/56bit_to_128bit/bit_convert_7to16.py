import os
import sys


def process_file(file_name):
    # 打开文件并读取内容
    with open(file_name, 'r') as file:
        content_of_file = file.read().splitlines()
    binary_rows = [[bin(int(char, 16))[2:].zfill(4) for char in row] for row in content_of_file]
    # print(binary_rows)
    binary_rows_final = [''.join(row) for row in binary_rows]
    # print(binary_rows_final)
    groups = [[row[i:i + 7] for i in range(0, len(row), 7)] for row in binary_rows_final]
    # print(groups)

    expanded_binary_rows = [[bit + '0' * 9 for bit in row] for row in groups]
    # print(expanded_binary_rows)
    hex_numbers = [[hex(int(bit, 2))[2:].zfill(4) for bit in row] for row in expanded_binary_rows]
    # print(hex_numbers)

    final_numbers = [''.join(row) for row in hex_numbers]
    # print(final_numbers)

    base_name, ext = os.path.splitext(input_file)
    output_base_name = base_name + "_16"
    output_file = output_base_name + ext
    with open(output_file, 'w') as file:
        file.write('\n'.join(final_numbers))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <input_file>")
        sys.exit(1)

    input_file = sys.argv[1]

    process_file(input_file)
