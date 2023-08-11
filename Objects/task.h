#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDebug>
#include "Objects/logger.h"
#include "QRegularExpression"


struct ConnectionString
{

    QString name;
    QString host;
    QString username;
    QString password;
    int port;
    QString Database;
};

enum EventTask
{
    LostConnect,
};

struct DataInterval{

    DataInterval(){
        Interval = "1";
        ByMonth = nullptr;
        ByWeekNumber = nullptr;
        ByYearDay = nullptr;
        ByMonthDay = nullptr;
        ByWeekDay = nullptr;
        ByHour = nullptr;
        ByMinute = nullptr;
        BySecond = nullptr;
        BySetpos = nullptr;
        ByDate = nullptr;
    }

    QString Freq;
    QString Interval;
    QString ByMonth;
    QString ByWeekNumber;
    QString ByYearDay;
    QString ByMonthDay;
    QString ByWeekDay;
    QString ByHour;
    QString ByMinute;
    QString BySecond;
    QString BySetpos;
    QString ByDate;

    float period(){
        if(Freq.toLower() == "weekly")
        {
            return 1.0f*7;
        }
        if(Freq.toLower() == "daily")
        {
            return 1.0f;
        }
        if(Freq.toLower() == "hourly")
        {
            return 1.0f/24;
        }
        if(Freq.toLower() == "minutely")
        {
            return 1.0f/24/60;
        }
        if(Freq.toLower() == "secondly")
        {
            return 1.0f/24/60/60;
        }
    }

    QDateTime getNextStart(QDateTime lastRun)
    {
        QDateTime result = lastRun;

        if(Freq.toLower() != "monthly" || Freq.toLower() != "yearly")
        {
            if(ByMonth != nullptr)
            {

            }
            if(ByWeekNumber != nullptr)
            {

            }
            if(ByYearDay != nullptr)
            {

            }
            if(ByMonthDay != nullptr)
            {

            }
            if(ByWeekDay != nullptr)
            {

            }
            if(Freq.toLower() != "secondly" && Freq.toLower() != "minutely" && Freq.toLower() != "hourly")
            {
                if(ByHour != nullptr)
                {
                    QTime time = result.time();
                    time.setHMS(ByHour.toInt(), time.minute(), time.second());
                    result.setTime(time);
                }
                else
                {
                    QTime time = result.time();
                    time.setHMS(0, time.minute(), time.second());
                    result.setTime(time);
                }
                if(ByMinute != nullptr)
                {
                    QTime time = result.time();
                    time.setHMS(time.hour(), ByMinute.toInt(), time.second());
                    result.setTime(time);
                }
                else
                {
                    QTime time = result.time();
                    time.setHMS(time.hour(), 0, time.second());
                    result.setTime(time);
                }

                if(BySecond != nullptr)
                {
                    QTime time = result.time();
                    time.setHMS(time.hour(), time.minute(), BySecond.toInt());
                    result.setTime(time);
                }
                else
                {
                    QTime time = result.time();
                    time.setHMS(time.hour(), time.minute(), 0);
                    result.setTime(time);
                }
            }
            if(BySetpos != nullptr)
            {

            }
            if(ByDate != nullptr)
            {

            }

            if(result <= QDateTime::currentDateTime())
            {
                if(Freq.toLower() == "weekly")
                {
                    result = lastRun.addDays(7*1*Interval.toInt());
                    //return 1.0f*7;
                }
                if(Freq.toLower() == "daily")
                {

                    result = lastRun.addDays(1*Interval.toInt());
                    //return 1.0f;
                }
                if(Freq.toLower() == "hourly")
                {
                    result = lastRun.addSecs(60*60*Interval.toInt());
                    //return 1.0f/24;
                }
                if(Freq.toLower() == "minutely")
                {
                    result = lastRun.addSecs(60*Interval.toInt());
                    //return 1.0f/24/60;
                }
                if(Freq.toLower() == "secondly")
                {
                    result = lastRun.addSecs(1*Interval.toInt());
                    //return 1.0f/24/60/60;
                }
            }

        }
        return result;
    }
};

class Task : public QObject
{
    Q_OBJECT
public:
    explicit Task(QObject *parent = nullptr);
    ~Task(){this->thread()->exit();}
    int id() const;
    void setId(int newId);
    QString name() const;
    void setName(const QString &newName);
    QString type() const;
    void setType(const QString &newType);
    QString action() const;
    void setAction(const QString &newAction);
    QDateTime start_date() const;
    void setStart_date(const QDateTime &newStart_date);
    int interval() const;
    void setInterval(int newInterval);
    QDateTime end_date() const;
    void setEnd_date(const QDateTime &newEnd_date);
    QString className() const;
    void setClassName(const QString &newClass);
    QString comment() const;
    void setComment(const QString &newComment);
    bool enabled_job() const;
    void setEnabled_job(bool newEnabled_job);
    bool auto_drop_job() const;
    void setAuto_drop_job(bool newAuto_drop_job);
    QString comments_job() const;
    void setComments_job(const QString &newComments_job);
    int status() const;
    void setStatus(int newStatus);
    QString str_interval() const;
    void setStr_interval(const QString &newStr_interval);
    QDateTime getLastRun() const;
    void setLastRun(const QDateTime &newLastRun);
    DataInterval getDataInterval() const;
    void setDataInterval(const DataInterval &newDataInterval);

    void update();



    bool checkCorrectDateTimeStart(QDateTime nextRun);

    bool isStartedInterval();
    void getNextStart();

    void parseIntervalToDataInterval();

    void saveLogMessageDB(QString message, QSqlDatabase db);

    ConnectionString getConnStr() const;
    void setConnStr(const ConnectionString &newConnStr);

    bool CheckLastRunFromPeriod(QDateTime lastRun);

    void CloseConnection(QSqlDatabase db);
    QSqlDatabase OpenConnection();

private:
        int _id;
        QString _name;
        QString _type;
        QString _action;
        QDateTime _start_date;
        int _interval;
        QDateTime _end_date;
        QString _className;
        QString _comment;
        bool _enabled_job;
        bool _auto_drop_job;
        QString _comments_job;
        int _status;
        QString _str_interval;
        DataInterval dataInterval;
        QDateTime lastRun;
        QDateTime nextRun;
        ConnectionString ConnStr;
        bool _saveLogDB;



signals:
    void finished();
    void UpdateLastRun(QDateTime , int);
    void SystemEvent(EventTask);
public slots:
    void run();

};

#endif // TASK_H
