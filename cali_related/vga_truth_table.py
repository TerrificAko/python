def update_vga_words(calib_num, calib_dir, cali_word):
    if calib_num == 0:
        cali_word = 0b100000

    elif calib_num == 1:
        cali_word_tmp = cali_word
        change_bit = ((0b1 & (cali_word_tmp >> (6 - calib_num))) ^ calib_dir)
        cali_word = modify_bit(cali_word_tmp, 6 - calib_num, change_bit)
        cali_word = modify_bit(cali_word, 5 - calib_num, 1)
    elif calib_num == 2:
        cali_word_tmp = cali_word
        change_bit = ((0b1 & (cali_word_tmp >> (6 - calib_num))) ^ calib_dir)
        cali_word = modify_bit(cali_word_tmp, 6 - calib_num, change_bit)
        cali_word = modify_bit(cali_word, 5 - calib_num, 1)
    elif calib_num == 3:
        cali_word_tmp = cali_word
        change_bit = ((0b1 & (cali_word_tmp >> (6 - calib_num))) ^ calib_dir)
        cali_word = modify_bit(cali_word_tmp, 6 - calib_num, change_bit)
        cali_word = modify_bit(cali_word, 5 - calib_num, 1)
    elif calib_num == 4:
        cali_word_tmp = cali_word
        change_bit = ((0b1 & (cali_word_tmp >> (6 - calib_num))) ^ calib_dir)
        cali_word = modify_bit(cali_word_tmp, 6 - calib_num, change_bit)
        cali_word = modify_bit(cali_word, 5 - calib_num, 1)
    elif calib_num == 5:
        if calib_dir:
            cali_word = (cali_word - 1) & 0b111111
        else:
            cali_word = (cali_word + 1) & 0b111111

    return cali_word




def modify_bit(value, position, new_value):
    # 确保 value 是一个 6 位的无符号整数
    value &= 0b111111  # 取模操作

    # 创建一个掩码，用来清除第 position 位
    clear_mask = ~(1 << position) & 0b111111  # 确保掩码也是 6 位
    # 清除第 position 位
    cleared_value = value & clear_mask

    # 创建一个掩码，用来设置第 position 位
    set_mask = new_value << position
    # 确保 set_mask 也在 6 位的范围内
    set_mask &= 0b111111

    # 设置第 position 位
    modified_value = cleared_value | set_mask
    # 返回 6 位的无符号整数
    return modified_value & 0b111111
def generate_truth_table():
    truth_table = []
    cali_word = 0b100000
    for table_input in range(64):  # 6-bit input
        # if table_input == 255:
            for calib_num in range(6):
                calib_dir = bool(table_input & (1 << (5 - calib_num)))
                cali_word = update_vga_words(calib_num, calib_dir, cali_word)

            truth_table.append((table_input, cali_word))

    return truth_table


truth_table = generate_truth_table()
with open('vga_truth_table_output.txt', 'w') as file:
    for entry in truth_table:
        print(f"Input: {entry[0]:06b} |Input 5bit: x{entry[0]&0b11111:05b}| cali_word: {entry[1]:06b} {entry[1]}")
        output_line = f"Input: {entry[0]:06b} |Input 5bit: x{entry[0]&0b11111:05b}| cali_word: {entry[1]:06b} {entry[1]} \n"
        file.write(output_line)

