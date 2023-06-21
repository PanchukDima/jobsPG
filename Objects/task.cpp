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

                lastRun = dataInterval.getNextStart(lastRun);

            }

        }
        else
        {
            nextRun = dataInterval.getNextStart(lastRun);
        }
    }

}



bool Task::isStartedInterval()
{

    if(start_date() < getLastRun())
    {

        if(CheckLastRunFromPeriod(getLastRun()) && getLastRun()>nextRun)
        {
            nextRun = getDataInterval().getNextStart(getLastRun());
        }
        if(nextRun<QDateTime::currentDateTime())
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

}

void Task::parseIntervalToDataInterval()
{
    DataInterval result;

    QStringList interval_arr = str_interval().split(";");
    foreach (QString interval_param, interval_arr) {

        QRegularExpression re("(.*)=(.*)");
        QRegularExpressionMatch match = re.match(interval_param);

        qDebug(logInfo())<<match.captured(1)<<" "<<match.captured(2).toLower();
        if(match.captured(1).toLower()== "freq")
        {
            result.Freq = match.captured(2).toLower();
        }
        if(match.captured(1).toLower() == "interval")
        {
            result.Interval = match.captured(2).toLower();
        }

        if(match.captured(1).toLower() == "byday")
        {
            result.ByWeekDay = match.captured(2).toLower();
        }
        if(match.captured(1).toLower() == "byhour")
        {
            result.ByHour = match.captured(2).toLower();
        }
        if(match.captured(1).toLower() == "byminute")
        {
            result.ByMinute = match.captured(2).toLower();
        }
        if(match.captured(1).toLower() == "bysecond")
        {
            result.BySecond = match.captured(2).toLower();
        }
    }

    setDataInterval(result);
}

void Task::saveLogMessageDB(QString message, QSqlDatabase db)
{

    if(db.open())
    {
       QSqlQuery *query = new QSqlQuery(db);
       query->exec("INSERT INTO dbms_scheduler.log_jobs (job_id, log_text) VALUES("+QString::number(_id)+", '"+message.simplified()+"')");
       if(query->lastError().isValid())
       {
           qDebug(logCritical())<<__FILE__<<__LINE__ <<"save log error SqlError"<< query->lastError().text() <<" "<<query->lastQuery();
       }
    }
}

ConnectionString Task::getConnStr() const
{
    return ConnStr;
}

void Task::setConnStr(const ConnectionString &newConnStr)
{
    ConnStr = newConnStr;
}

bool Task::CheckLastRunFromPeriod(QDateTime lastRun)
{

    QString Freq = getDataInterval().Freq.toLower();

    QTime curTime = QTime::currentTime();
    QDateTime curDateTime = QDateTime::currentDateTime();
    if(Freq.toLower() == "weekly")
    {
        return false;
    }
    if(Freq.toLower() == "daily")
    {
        QTime startTime = curTime;
        QTime endTime = curTime;
        startTime.setHMS(0,0,0);
        endTime.setHMS(23,59,59);
        QDateTime start = curDateTime;
        QDateTime end = curDateTime;
        start.setTime(startTime);
        end.setTime(endTime);

        if((lastRun > start || lastRun < end))
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    if(Freq.toLower() == "hourly")
    {
        return false;
    }
    if(Freq.toLower() == "minutely")
    {
        return false;
    }
    if(Freq.toLower() == "secondly")
    {
        return false;
    }
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
    nextRun = getDataInterval().getNextStart(newLastRun);
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
    QString application_name = "application_name=Job_"+getConnStr().name;
    ThreadDB.setConnectOptions(application_name);

    if(ThreadDB.open()){
        saveLogMessageDB("Job Start", ThreadDB);
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
                        while(!db.open()){
                            if (db.open()) {
                                qDebug(logInfo()) << "Reconnected!";
                            } else {
                                qDebug(logCritical()) <<__FILE__<<__LINE__ << "Could not reconnect to the database. Next try in 2 seconds....";
                            }
                            QThread::sleep(2);
                        }
                        emit SystemEvent(EventTask::LostConnect);
                    }
                    else
                    {
                        setLastRun(QDateTime::currentDateTime());
                        QSqlQuery query =  QSqlQuery(db);
                        qDebug(logDebug())<<"Sql execute: "<<action();
                        query.exec("UPDATE dbms_scheduler.jobs SET pid=pg_backend_pid() WHERE id="+QString::number(_id)+";");
                        query.exec(action());

                        if(query.lastError().isValid())
                        {
                            qDebug(logCritical())<<__FILE__<<__LINE__ <<getConnStr().name<<"SQL Error:"<<query.lastError();
                            qDebug(logCritical())<<__FILE__<<__LINE__ <<query.lastError().nativeErrorCode();
                            saveLogMessageDB(query.lastError().databaseText(), ThreadDB);
                            //emit SystemEvent(EventTask::LostConnect);
                            continue;
                        }
                        query.exec("UPDATE dbms_scheduler.jobs SET pid=NULL WHERE id="+QString::number(_id)+";");
                        if(query.lastError().isValid())
                        {
                            qDebug(logCritical())<<__FILE__<<__LINE__ <<getConnStr().name<<"SQL Error:"<<query.lastError();
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
        while(!ThreadDB.open()){
            if (ThreadDB.open()) {
                qDebug(logInfo()) << "Reconnected!";
            } else {
                qDebug(logCritical())<<__FILE__<<__LINE__  << "Could not reconnect to the database. Next try in 2 seconds";
            }
            QThread::sleep(2);
        }
        emit SystemEvent(EventTask::LostConnect);
    }
}


