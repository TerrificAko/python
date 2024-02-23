# 输入任意个数，以end结尾，将这些数从小到大排序 最后输出

value = []

while 1:
    input_str = input("输入：")

    if input_str == "end":
        break

    value.append(float(input_str))

sorted_value = sorted(value)

print("最后排序如下")
print(*sorted_value, sep=",")
