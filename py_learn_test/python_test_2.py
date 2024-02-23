# 利润与奖金的关系 阶梯式
# 首先要有输入的利润
i = int(input('净利润 ：'))

#分成划分区间及比例
profit = [1000000,600000,400000,200000,100000,0]
ratio = [0.01,0.015,0.03,0.05,0.075,0.1]
bonus =0
for idx in range(0,6):
    if i>profit[idx] :
        bonus += (i-profit[idx])*ratio[idx]
        print(f"阶段{idx},累积奖金{bonus}")
        i = profit[idx]

print('共计奖金：',bonus)
print(f"{'Hello':>20s}")


