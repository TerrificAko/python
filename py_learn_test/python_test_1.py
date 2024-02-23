# 1-4 互不重复的三位数 能出多少个并给出最后计数
match_times = 0
match_list = []
unmatch_times = 0
unmatch_list = []
for i in range(1, 5):
    for j in range(1, 5):
        for k in range(1, 5):
            a = i * 100 + j * 10 + k
            if i == k or i == j or j == k:
                unmatch_list.append(a)
                unmatch_times += 1
            else:
                match_list.append(a)
                match_times += 1

print('匹配列表', match_list)
print('匹配个数', match_times)
print('不匹配列表', unmatch_list)
print('不匹配个数', unmatch_times)
