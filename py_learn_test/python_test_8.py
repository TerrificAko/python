# 输出 9x9 乘法口诀表 同时验证延时功能 延时输出暂停一秒输出

for i in range(1, 10):
    print(" ")
    for j in range(1, i + 1):
        # print(f"{i}*{j}={i*j}", end=" ")
        # print("%d*%d=%d" % (i, j, i * j), end=" ")
        print(f"{i:d}*{j:d}={i * j:d}", end=" ")

import time

# 字典
dictionary_value = {"面向": "对象", "chmod": "777"}

for key, value in dict.items(dictionary_value):
    print(key, value)
    time.sleep(1)

print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time())))
time.sleep(2)
print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time())))
