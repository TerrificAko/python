# 斐波那契数列
# F0 = 0     (n=0)
# F1 = 1    (n=1)
# Fn = F[n-1]+ F[n-2](n=>2)

def fib(n):
    if n == 1:
        return [1]

    if n == 2:
        return [1, 1]

    fib_list = [1, 1]
    # i=3
    # while i <= n:
    #     fib_list.append(fib_list[-1] + fib_list[-2])
    #     i += 1

    for i in range(2, n):
        fib_list.append(fib_list[-1] + fib_list[-2])

    return fib_list


value = input("请输入你想展现的斐波那契数列的个数：")
value_int = int(value)
print(fib(value_int))


def rabbit(n):
    count = [1,0,0]  #将兔子成长期分为三个月
    for i in range(1,n): #每个月更新一次不同成长期的兔子对数
        count[2] = count[2] + count[1]
        count[1] = count[0]
        count[0] = count[2]
    return count[0]+count[1]+count[2] #返回兔子对数总数

n = int(input("查看第几个月的兔子对数："))
rabbit_sum = rabbit(n)
print("第%d个月的兔子对数为%d"%(n,rabbit_sum))