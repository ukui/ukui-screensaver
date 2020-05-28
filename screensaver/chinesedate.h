#ifndef CHINESEDATE_H
#define CHINESEDATE_H
#include <QString>
#include <QStringList>

class ChineseDate
{
public:
    ChineseDate();
    QString getDateLunar();
private:
    /**
     * @brief 根据农历年份获取该年天数
     * @param year     农历年份
     * @return
     */
    int getYearDays(int year);
    /**
     * @brief 根据农历年份判断该年是否有闰月
     * @param year     农历年份
     * @return
     */
    bool isLeapMonth(int year);
    /**
     * @brief 根据农历年份获取闰月的天数，如果存在闰月的话
     * @param year     农历年份
     * @return
     */
    int getLeapMonthDays(int year);
    /**
     * @brief 根据农历年份获取闰月的月份
     * @param year     农历年份
     * @return
     */
    int getLeapMonth(int year);
    /**
     * @brief 根据农历年份和月份获取当月的天数
     * @param year     农历年份
     * @param month    农历月份
     * @return
     */
    int getYearMonthDays(int year,int month);
    /**
     * @brief 获取农历日期
     * @return
     */
    QString getChineseDays(int month,int day,bool isleap);

    QStringList ChineseMonth;
    QStringList ChineseDay;
    QStringList ChineseDays;

};

#endif // CHINESEDATE_H
