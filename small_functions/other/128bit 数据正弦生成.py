import math


# def hex_complement_conversion(16bit_value):
amplitude = 8192


for i in range(1024):
    value = round(amplitude * math.sin(2 * math.pi * i / 1024))
    print(value)
    value_bin = bin(value&0xFFFF)
    print(value_bin)
    value_bin = int(value_bin[2:],base = 2)
    value_hex = hex(value_bin)# 补码值
    print(value_hex)

    # vlaue_hex = hex(int(bin(int(value) & 0xFFFF)))
    # if value < 0:
    #     vlaue_hex = hex(bin(value & 0xFFFF))
    # else:
    #     value_hex = hex(value)
    # print(value_hex)
    # data.append(value)





#
#
# class WaveGen:
#     def __init__(self,output_file,amplitude,depth,num_cycles):
#         self.output_file = output_file
#         self.amplitude = amplitude
#         self.depth = depth
#         self.num_cycles = num_cycles
#
#     def generate_data(self):
#         value = []
#         value =
#
#     def write_data(self):
#         pass
#
# def main():
#     wave_gen = WaveGen(output_file)
#
#
#
# if __name__ == '__main__':
#     main();