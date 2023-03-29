#include "task.h"

Task::Task(QObject *parent)
    : QObject{parent}
{

}

int Task::id() const
{
    return _id;
}

void Task::setId(int newId)
{
    _id = newId;
}

QString Task::name() const
{
    return _name;
}

void Task::setName(const QString &newName)
{
    _name = newName;
}

QString Task::type() const
{
    return _type;
}

void Task::setType(const QString &newType)
{
    _type = newType;
}

QString Task::action() const
{
    return _action;
}

void Task::setAction(const QString &newAction)
{
    _action = newAction;
}

QDateTime Task::start_date() const
{
    return _start_date;
}

void Task::setStart_date(const QDateTime &newStart_date)
{
    _start_date = newStart_date;
}

int Task::interval() const
{
    return _interval;
}

void Task::setInterval(int newInterval)
{
    _interval = newInterval;
}

QDateTime Task::end_date() const
{
    return _end_date;
}

void Task::setEnd_date(const QDateTime &newEnd_date)
{
    _end_date = newEnd_date;
}

QString Task::className() const
{
    return _className;
}

void Task::setClassName(const QString &newClass)
{
    _className = newClass;
}

QString Task::comment() const
{
    return _comment;
}

void Task::setComment(const QString &newComment)
{
    _comment = newComment;
}

bool Task::enabled_job() const
{
    return _enabled_job;
}

void Task::setEnabled_job(bool newEnabled_job)
{
    _enabled_job = newEnabled_job;
    qDebug(logInfo())<<"status change";
}

bool Task::auto_drop_job() const
{
    return _auto_drop_job;
}

void Task::setAuto_drop_job(bool newAuto_drop_job)
{
    _auto_drop_job = newAuto_drop_job;
}

QString Task::comments_job() const
{
    return _comments_job;
}

void Task::setComments_job(const QString &newComments_job)
{
    _comments_job = newComments_job;
}

int Task::status() const
{
    return _status;
}

void Task::setStatus(int newStatus)
{
    _status = newStatus;
}

void Task::update()
{
    QSqlQuery query;
    query.exec("SELECT "
               " j.name,"
               " j.type_job,"
               " j.action,"
               " j.startdate,"
               " j.repeat_interval_job,"
               " j.enddate,"
               " j.class,"
               " j.enabled_job,"
               " j.auto_drop_job,"
               " j.comments_job,"
               " j.status,"
               " j.str_interval,"
               " j.last_run"
               " FROM dbms_scheduler.jobs j WHERE id ="+QString::number(_id));
    while(query.next())
    {
        _name = query.value(0).toString();
        _type = query.value(1).toString();
        _action = query.value(2).toString();
        _start_date = query.value(3).toDateTime();
        _interval = query.value(4).toInt();
        _end_date = query.value(5).toDateTime();
        _className = query.value(6).toString();
        _enabled_job = query.value(7).toBool();
        _auto_drop_job = query.value(8).toBool();
        _comment = query.value(9).toString();
        _status = query.value(10).toInt();
        _str_interval = query.value(11).toString();
        lastRun = query.value(12).toDateTime();
        ConnStr.name = _name;
        parseIntervalToDataInterval();
        if(lastRun.isNull())
        {
            lastRun = _start_date;

            if(lastRun.daysTo(QDateTime::currentDateTime()) > 365)
            {
                lastRun = QDateTime::currentDateTime().addDays(-1);

            }
            while(lastRun < QDateTime::currentDateTime())
            {
                qDebug()<<QDateTime::currentDateTime();
                qDebug()<<lastRun;
                lastRun = dataInterval.getNextStart(lastRun);

            }

        }
        else
        {
            while(lastRun < QDateTime::currentDateTime())
            {
             lastRun = dataInterval.getNextStart(lastRun);
            }
        }
    }

}



bool Task::isStartedInterval()
{

    if(start_date() < getLastRun())
    {
        QDateTime NextDateTimeStart = getDataInterval().getNextStart(getLastRun());
        if(NextDateTimeStart<QDateTime::currentDateTime())
        {            
                return true;
        }
    }
    else
    {
        return false;
    }
    return false;
}

void Task::getNextStart()
{
    qDebug()<<getDataInterval().getNextStart(getLastRun());
}

void Task::parseIntervalToDataInterval()
{
    DataInterval result;

    QStringList interval_arr = str_interval().split(";");
    foreach (QString interval_param, interval_arr) {

        QRegularExpression re("(.*)=(.*)");
        QRegularExpressionMatch match = re.match(interval_param);

        qDebug()<<match.captured(1)<<" "<<match.captured(2).toLower();
        if(match.captured(1) == "Freq")
        {
            result.Freq = match.captured(2).toLower();
        }
        if(match.captured(1) == "Interval")
        {
            result.Interval = match.captured(2).toLower();
        }

        if(match.captured(1) == "ByDay")
        {
            result.ByWeekDay = match.captured(2).toLower();
        }
        if(match.captured(1) == "ByHour")
        {
            result.ByHour = match.captured(2).toLower();
        }
        if(match.captured(1) == "ByMinute")
        {
            result.ByMinute = match.captured(2).toLower();
        }
        if(match.captured(1) == "BySecond")
        {
            result.BySecond = match.captured(2).toLower();
        }
    }

    setDataInterval(result);
}

ConnectionString Task::getConnStr() const
{
    return ConnStr;
}

void Task::setConnStr(const ConnectionString &newConnStr)
{
    ConnStr = newConnStr;
}

DataInterval Task::getDataInterval() const
{
    return dataInterval;
}

void Task::setDataInterval(const DataInterval &newDataInterval)
{
    dataInterval = newDataInterval;
}

QDateTime Task::getLastRun() const
{
    return lastRun;
}

void Task::setLastRun(const QDateTime &newLastRun)
{
    lastRun = newLastRun;
    emit UpdateLastRun(lastRun, _id);

}

QString Task::str_interval() const
{
    return _str_interval;
}

void Task::setStr_interval(const QString &newStr_interval)
{
    _str_interval = newStr_interval;
}

void Task::run()
{
    qDebug(logInfo())<<"Job id: "<<QString::number(_id)<<"Enabled job "<<_enabled_job;
    QSqlDatabase ThreadDB = QSqlDatabase::addDatabase("QPSQL", getConnStr().name);
    ThreadDB.setHostName(getConnStr().host);
    ThreadDB.setUserName(getConnStr().username);
    ThreadDB.setPassword(getConnStr().password);
    ThreadDB.setPort(getConnStr().port);
    ThreadDB.setDatabaseName(getConnStr().Database);
    if(ThreadDB.open()){
        while(true){
            while(_enabled_job)
            {
                QThread::sleep(1);
                if(isStartedInterval())
                {                    
                    qDebug(logInfo())<<"Execute action task id: "<<QString::number(_id)<<"DateTime Start:"<<QDateTime::currentDateTime();
                    setLastRun(QDateTime::currentDateTime());
                    QSqlDatabase db = QSqlDatabase::database(getConnStr().name);
                    if(!db.open())
                    {
                        qDebug(logInfo()) << "Trying to reconnect...";
                        db.close();
                        if (db.open()) {
                            qDebug(logInfo()) << "Reconnected!";
                        } else {
                            qDebug(logCritical()) << "Could not reconnect to the database.";
                        }
                        emit SystemEvent(EventTask::LostConnect);
                    }
                    else
                    {
                        QSqlQuery query =  QSqlQuery(db);
                        qDebug(logDebug())<<"Sql execute: "<<action();
                        query.exec("UPDATE dbms_scheduler.jobs SET pid=pg_backend_pid() WHERE id="+QString::number(_id)+";");
                        query.exec(action());

                        if(query.lastError().isValid())
                        {
                            qDebug(logCritical())<<getConnStr().name<<"SQL Error:"<<query.lastError();
                            qDebug(logCritical())<<query.lastError().nativeErrorCode();
                            emit SystemEvent(EventTask::LostConnect);
                            continue;
                        }
                        query.exec("UPDATE dbms_scheduler.jobs SET pid=NULL WHERE id="+QString::number(_id)+";");
                        if(query.lastError().isValid())
                        {
                            qDebug(logCritical())<<getConnStr().name<<"SQL Error:"<<query.lastError();
                        }
                    }

                    qDebug(logInfo())<<"end Work task id: "<<QString::number(_id)<<"DateTime End:"<<QDateTime::currentDateTime();

                }
            }
        }
    }
    else
    {
        qDebug(logInfo()) << "Trying to reconnect...";
        ThreadDB.close();
        if (ThreadDB.open()) {
            qDebug(logInfo()) << "Reconnected!";
        } else {
            qDebug(logCritical()) << "Could not reconnect to the database.";
        }
        emit SystemEvent(EventTask::LostConnect);
    }
}


