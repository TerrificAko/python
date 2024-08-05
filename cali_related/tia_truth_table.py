def update_tia_words(calib_num, tia_comp_dir, tia_word_p, tia_word_n):
    if calib_num == 0:
        tia_word_p = 0b1000000
        tia_word_n = 0b0111111
    elif calib_num == 1:
        tia_word_p_tmp = tia_word_p
        tia_word_n_tmp = tia_word_n
        change_bit = ((0b1 & (tia_word_p_tmp >> (7 - calib_num))) ^ tia_comp_dir)
        tia_word_p = modify_bit(tia_word_p_tmp, 7-calib_num, change_bit)
        tia_word_p = modify_bit(tia_word_p, 6-calib_num, 1)
        tia_word_n = modify_bit(tia_word_n_tmp, 7-calib_num, ~change_bit & 1)
        tia_word_n = modify_bit(tia_word_n, 6-calib_num, 0)


    elif calib_num == 2:
        tia_word_p_tmp = tia_word_p
        tia_word_n_tmp = tia_word_n
        change_bit = ((0b1 & (tia_word_p_tmp >> (7 - calib_num))) ^ tia_comp_dir)
        tia_word_p = modify_bit(tia_word_p_tmp, 7 - calib_num, change_bit)
        tia_word_p = modify_bit(tia_word_p, 6 - calib_num, 1)
        tia_word_n = modify_bit(tia_word_n_tmp, 7 - calib_num, ~change_bit & 1)
        tia_word_n = modify_bit(tia_word_n, 6 - calib_num, 0)
    elif calib_num == 3:
        tia_word_p_tmp = tia_word_p
        tia_word_n_tmp = tia_word_n
        change_bit = ((0b1 & (tia_word_p_tmp >> (7 - calib_num))) ^ tia_comp_dir)
        tia_word_p = modify_bit(tia_word_p_tmp, 7 - calib_num, change_bit)
        tia_word_p = modify_bit(tia_word_p, 6 - calib_num, 1)
        tia_word_n = modify_bit(tia_word_n_tmp, 7 - calib_num, ~change_bit & 1)
        tia_word_n = modify_bit(tia_word_n, 6 - calib_num, 0)

    elif calib_num == 4:
        tia_word_p_tmp = tia_word_p
        tia_word_n_tmp = tia_word_n
        change_bit = ((0b1 & (tia_word_p_tmp >> (7 - calib_num))) ^ tia_comp_dir)
        tia_word_p = modify_bit(tia_word_p_tmp, 7 - calib_num, change_bit)
        tia_word_p = modify_bit(tia_word_p, 6 - calib_num, 1)
        tia_word_n = modify_bit(tia_word_n_tmp, 7 - calib_num, ~change_bit & 1)
        tia_word_n = modify_bit(tia_word_n, 6 - calib_num, 0)

    elif calib_num == 5:
        tia_word_p_tmp = tia_word_p
        tia_word_n_tmp = tia_word_n
        change_bit = ((0b1 & (tia_word_p_tmp >> (7 - calib_num))) ^ tia_comp_dir)
        tia_word_p = modify_bit(tia_word_p_tmp, 7 - calib_num, change_bit)
        tia_word_p = modify_bit(tia_word_p, 6 - calib_num, 1)
        tia_word_n = modify_bit(tia_word_n_tmp, 7 - calib_num, ~change_bit & 1)
        tia_word_n = modify_bit(tia_word_n, 6 - calib_num, 0)

    elif calib_num == 6:
        if tia_comp_dir:
            tia_word_p = (tia_word_p - 1) & 0b1111111
        else:
            tia_word_p = (tia_word_p + 1) & 0b1111111
    elif calib_num == 7:
        if tia_comp_dir:
            tia_word_n = (tia_word_n + 1) & 0b1111111
        else:
            tia_word_n = (tia_word_n - 1) & 0b1111111

    return tia_word_p, tia_word_n


def calculate_diff(tia_word_p, tia_word_n):
    # Calculate the difference using 8 bits (including sign bit)
    diff = (tia_word_p - tia_word_n)  # Use 8 bits, 0xFF is 8-bit mask
    # Convert to signed integer if needed

    return diff


def modify_bit(value, position, new_value):
    # 确保 value 是一个 7 位的无符号整数
    value &= 0b1111111  # 取模操作

    # 创建一个掩码，用来清除第 position 位
    clear_mask = ~(1 << position) & 0b1111111  # 确保掩码也是 7 位
    # 清除第 position 位
    cleared_value = value & clear_mask

    # 创建一个掩码，用来设置第 position 位
    set_mask = new_value << position
    # 确保 set_mask 也在 7 位的范围内
    set_mask &= 0b1111111

    # 设置第 position 位
    modified_value = cleared_value | set_mask
    # 返回 7 位的无符号整数
    return modified_value & 0b1111111
def generate_truth_table():
    truth_table = []
    tia_word_p = 0b1000000
    tia_word_n = 0b0111111
    for table_input in range(256):  # 8-bit input
        # if table_input == 255:


            for calib_num in range(8):
                tia_comp_dir = bool(table_input & (1 << (7 - calib_num)))
                tia_word_p, tia_word_n = update_tia_words(calib_num, tia_comp_dir, tia_word_p, tia_word_n)

            diff = calculate_diff(tia_word_p, tia_word_n)
            truth_table.append((table_input, tia_word_p, tia_word_n, diff))

    return truth_table


truth_table = generate_truth_table()
with open('tia_truth_table_output.txt', 'w') as file:
    for entry in truth_table:  # Show only the first 5 entries for brevity
        print(f"Input: {entry[0]:08b} |Input 7bit: x{entry[0]&0b1111111:07b}| tia_word_p: {entry[1]:07b} | tia_word_n: {entry[2]:07b} | Diff: {entry[3]}")
        output_line = f"Input: {entry[0]:08b} |Input 7bit: x{entry[0]&0b1111111:07b}| tia_word_p: {entry[1]:07b} | tia_word_n: {entry[2]:07b} | Diff: {entry[3]}\n"
        file.write(output_line)

