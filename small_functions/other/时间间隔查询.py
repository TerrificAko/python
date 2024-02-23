from datetime import datetime, timedelta


class DateInterval:
    def __init__(self, start_date, end_date):
        self.start_date = datetime.strptime(start_date, '%Y-%m-%d')
        self.end_date = datetime.strptime(end_date, '%Y-%m-%d')

    def get_interval_days(self):
        interval = self.end_date - self.start_date
        return interval.days


# 示例用法
print("格式为\"2023-02-14\" , or now")
start_date = input("请输入起始日期:")
if start_date == "now":
    start_date = datetime.now().strftime('%Y-%m-%d')
    print("now is ", start_date)

end_date = input("请输入结束日期:")
if end_date == "now":
    end_date = datetime.now().strftime('%Y-%m-%d')
    print("now is ", end_date)

date_interval = DateInterval(start_date, end_date)
print(date_interval.get_interval_days())
