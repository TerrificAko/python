import sys

date_input_16bit = input("输入量化后的正值")


date_16bit_value = int(date_input_16bit)
data_7bit_value = (date_16bit_value / (2**15)) * 128

print(f"data_7bit_value is {data_7bit_value}")