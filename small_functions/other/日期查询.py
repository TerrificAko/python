# 一个整数，它加上100后是一个完全平方数，再加上168又是一个完全平方数，请问该数是多少 没有做

# 输入某年某月某日，输出为该年的第几天
# 闰年二月多一天
# 1、3、5、7、8、10、12 为31
# 2 为 28或29
# 4、6、9、11 为30

import sys

month_list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
month_list_31 = [1, 3, 5, 7, 8, 10, 12]
month_list_30 = [4, 6, 9, 11]

date = input("年月日的格式为\"19970606\"\n请输入日期:")
year = int(date[:4])
month = int(date[4:6])
day = int(date[6:])

leap = 0
if (year % 400 == 0) or ((year % 4 == 0) and (year % 100 != 0)):
    leap = 1

print(f"Year is {year}")
if month in month_list:
    print(f"Month is {month}")
else:
    print("这不是月份数值")
    sys.exit()

if ((month in month_list_31 and 0 < day < 32) or
    (month in month_list_30 and 0 < day < 31) or
    (leap == 0 and 0 < day < 29) or
    (leap == 1 and 0 < day < 30)):
    print(f"Day is {day}")
else:
    print("这不是日期值")
    sys.exit()


day_th = [0,31,59,90,120,151,181,212,243,273,304,334]
value = 0
if leap == 1 and month > 2:
    value = day + day_th[month-1] +1
else :
    value = day + day_th[month-1]

print(f"这是第{value}天")