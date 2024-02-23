import math
import matplotlib.pyplot as plt

# 生成正弦波数据
data = []
amplitude = 8191
for i in range(1024):
    # value = round(amplitude * math.sin(2 * math.pi * i / 1024))
    value = round(amplitude * math.sin( 6 * math.pi * i / 1024))
    data.append(value)

# 按组打印数据并绘制正弦波图像
plt.plot(data)
plt.title('Sinewave')
plt.xlabel('Sample')
plt.ylabel('Amplitude')

for i in range(128):
    group = data[i * 8: (i + 1) * 8]
    shifted_group = []
    for point in group:
        value = point << 2
        if value & 0x8000:
            value |= 0xffff0000  # 符号位扩展
        hex_string = format(value & 0xffff, '04X')
        shifted_group.append("16'h" + hex_string)

    shifted_group = shifted_group[::-1]  # 颠倒排序
    hex_group = ','.join(shifted_group)
    print(f"\'h{i:02x}:   dac_data    <= {{{hex_group}}};")

plt.show()
