#include "pgworker.h"

PGWorker::PGWorker(QObject *parent)
    : QObject{parent}
{

}

void PGWorker::init()
{
    deploy();
    QString fileConfig = getFileConfigPath();
    qDebug(logInfo())<<"File config is use"<<fileConfig;
    QSettings settings(fileConfig, QSettings::IniFormat);
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    bool eventNot= db.driver()->hasFeature(QSqlDriver::EventNotifications);
    qDebug(logInfo()) << "Has feature eventNotification" << eventNot;
    db.setHostName(settings.value("host").toString());
    db.setUserName(settings.value("Username").toString());
    db.setPassword(settings.value("password").toString());
    db.setPort(settings.value("port").toInt());
    db.setDatabaseName(settings.value("databasename").toString());
    db.setConnectOptions("application_name=JobsPG");
    if(db.open())
    {
        deployDB();
        qDebug(logInfo())<<"Connection true";
        QSqlDatabase::database().driver()->subscribeToNotification("dbms_sheduler_jobs");
        connect(QSqlDatabase::database().driver(),SIGNAL(notification(QString,QSqlDriver::NotificationSource,QVariant)),SLOT(NotifyHandler(QString,QSqlDriver::NotificationSource,QVariant)));
        getListTasks();
        startTasks();
    }
    else
    {
        qDebug(logCritical())<<__FILE__<<__LINE__ <<db.lastError();
    }
}

QString PGWorker::getFileConfigPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[1]+"/connection";
}



void PGWorker::getListTasks()
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query;
    query.exec("SELECT j.id FROM dbms_scheduler.jobs j;");
    while (query.next()) {
            WorkerObject *worker = new WorkerObject();
            worker->task = new Task();
            worker->task->setId(query.value(0).toInt());
            worker->task->update();
            ConnectionString connStr = worker->task->getConnStr();
            connStr.host = db.hostName();
            connStr.username = db.userName();
            connStr.password = db.password();
            connStr.port = db.port();
            connStr.Database = db.databaseName();
            worker->task->setConnStr(connStr);
            workers.append(worker);
            qDebug(logInfo())<<"Load Task id: "<<query.value(0).toString();

    }


}

void PGWorker::startTasks()
{

    foreach (WorkerObject * worker, workers) {
        qDebug(logInfo())<<"Create threads task name "<<worker->task->name();
        worker->thread = new QThread(this);
        worker->task->moveToThread(worker->thread);
        connect(worker->thread, SIGNAL(started()), worker->task,SLOT(run()));
        connect(worker->task,SIGNAL(UpdateLastRun(QDateTime,int)),SLOT(UpdateLastRunTask(QDateTime,int)));
        connect(worker->task,SIGNAL(finished()),worker->task,SLOT(deleteLater()));

        worker->thread->start();
    }

}

void PGWorker::startTask(int p_id)
{
    foreach (WorkerObject * worker, workers) {
        if(worker->task->id() == p_id)
        {
            worker->thread = new QThread(this);
            worker->task->moveToThread(worker->thread);
            connect(worker->thread, SIGNAL(started()), worker->task,SLOT(run()));
            connect(worker->task,SIGNAL(UpdateLastRun(QDateTime,int)),SLOT(UpdateLastRunTask(QDateTime,int)));
            worker->thread->start();
        }
    }

}

void PGWorker::deploy()
{
    QString fileConfigPath = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[1]+"/connection";
    QFile fileConfig(fileConfigPath);
    QDir configDir(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[1]);
    /*if(!configDir.exists())
    {
        QDir::mkdir(configDir.absolutePath());
    }*/
    if(!fileConfig.exists())
    {
        QSettings settings(fileConfigPath,QSettings::IniFormat);
        settings.setValue("host", "127.0.0.1");
        settings.setValue("Username", "SOLUTION_MED");
        settings.setValue("password", "elsoft");
        settings.setValue("port", 5432);
        settings.setValue("databasename", "med");
    }


}

void PGWorker::deployDB()
{
    QSqlQuery query;
    query.exec("select count(*) from pg_namespace where nspname = 'dbms_scheduler'");
    if(query.lastError().isValid())
    {
        qDebug(logCritical())<<__FILE__<<__LINE__ <<"Check exists DB schema 'dbms_scheduler' failed\n"<<query.lastError();
    }
    while(query.next())
    {
        if(query.value(0).toInt() == 0)
        {
            query.exec("CREATE SCHEMA dbms_scheduler AUTHORIZATION postgres;");
            if(query.lastError().isValid())
            {
                qDebug(logCritical())<<__FILE__<<__LINE__ <<"Create DB schema 'dbms_scheduler' failed\n"<<query.lastError();
            }
        }
        else
        {
            qDebug(logInfo())<<"DB schema 'dbms_scheduler' exists";
        }
    }
    query.exec("select count(*) from pg_tables pt where pt.schemaname = 'dbms_scheduler' and pt.tablename = 'jobs'");
    if(query.lastError().isValid())
        {
            qDebug(logCritical())<<__FILE__<<__LINE__ <<"Check exists DB table 'dbms_scheduler.jobs' failed\n"<<query.lastError();
        }
    while(query.next())
    {
        if(query.value(0).toInt() == 0)
        {
            qDebug(logInfo())<<"DB Trigger 'dbms_scheduler.trigger_event_jobs' NOT exist";
            qDebug(logInfo())<<"Deploy DB Table 'dbms_scheduler.jobs'";
            query.exec("CREATE TABLE dbms_scheduler.jobs ("
                       " id bigserial NOT NULL,  "
                       " \"name\" varchar NULL,  "
                       " type_job varchar NULL, "
                       " \"action\" varchar NULL, "
                       " startdate timestamp NULL, "
                       " repeat_interval_job int8 NULL, "
                       " enddate timestamp NULL DEFAULT now(), "
                       " \"class\" text NULL, "
                       " enabled_job bool NULL, "
                       " auto_drop_job bool NULL, "
                       " comments_job text NULL, "
                       " status int8 NULL DEFAULT 0, "
                       " last_run timestamp NULL,"
                       " str_interval varchar NULL,"
                       " pid bigint NULL,"
                       " CONSTRAINT jobs_pk PRIMARY KEY (id), "
                       " CONSTRAINT jobs_un UNIQUE (name) "
                   " );"
                       "create trigger trg_events_jobs after"
                       " insert"
                           " or"
                       " delete"
                           " or"
                       " update"
                           " on"
                           " dbms_scheduler.jobs for each row execute function dbms_scheduler.trigger_event_jobs();");
            if(query.lastError().isValid())
            {
                qDebug(logCritical())<<__FILE__<<__LINE__ <<"Create DB table 'dbms_scheduler.jobs' failed\n"<<query.lastError();
            }
        }
        else
        {
            query.exec("select count(*)  from information_schema.columns c where c.table_name= 'jobs' and c.table_schema = 'dbms_scheduler'");
            while(query.next())
            {
                if(query.value(0).toInt() == 15)
                {
                    qDebug(logInfo())<<"DB table 'dbms_scheduler.jobs' correct";
                }
                else
                {
                    qDebug(logCritical())<<__FILE__<<__LINE__ <<"DB table 'dbms_scheduler.jobs' incorrect? please update"<<query.value(0).toInt();
                    exit(0);
                }
            }
        }
    }
    query.exec("select count(*) from pg_proc pp, pg_namespace pn "
             "  where pn.\"oid\" = pp.pronamespace"
              " and pp.proname  = 'trigger_event_jobs'"
             "  and pn.nspname iLIKE  'dbms_scheduler'");
    if(query.lastError().isValid())
    {
        qDebug(logCritical())<<__FILE__<<__LINE__ <<"Check exists DB Trigger 'dbms_scheduler.trigger_event_jobs', failed";
    }
    while (query.next()) {
        if(query.value(0).toInt() == 1)
        {
             qDebug(logInfo())<<__FILE__<<__LINE__ <<"DB Trigger 'dbms_scheduler.trigger_event_jobs' exist";
        }
        else
        {
            qDebug(logInfo())<<"DB Trigger 'dbms_scheduler.trigger_event_jobs' NOT exist";
            qDebug(logCritical())<<__FILE__<<__LINE__ <<"Deploy DB Trigger 'dbms_scheduler.trigger_event_jobs'";
            exit(0);
        }

    }

}

void PGWorker::NotifyHandler(QString val, QSqlDriver::NotificationSource notifySource, QVariant msg)
{
    qDebug(logInfo())<<val<<" "<< notifySource <<" "<< msg;
    QSqlDatabase db = QSqlDatabase::database();
    QStringList command = msg.toString().split(";");
    if(command[0] == "UPDATE")
    {
        foreach (WorkerObject *worker, workers) {
            if(worker->task->id() == command[1].toInt())
            {
                worker->task->update();
            }
        }
    }
    if(command[0] == "STATE")
    {
        foreach (WorkerObject *worker, workers) {
            if(worker->task->id() == command[1].toInt())
            {
                QSqlQuery query;
                query.exec("SELECT enabled_job from dbms_scheduler.jobs WHERE id="+command[1]);
                while(query.next())
                {
                    worker->task->setEnabled_job(query.value(0).toBool());
                }
            }
        }
    }
    if(command[0] == "ADD")
    {
        WorkerObject *worker = new WorkerObject();
        worker->task = new Task();
        worker->task->setId(command[1].toInt());
        worker->task->update();
        ConnectionString connStr = worker->task->getConnStr();
        connStr.host = db.hostName();
        connStr.username = db.userName();
        connStr.password = db.password();
        connStr.port = db.port();
        connStr.Database = db.databaseName();
        worker->task->setConnStr(connStr);
        workers.append(worker);
        startTask(command[1].toInt());
    }


}

void PGWorker::UpdateLastRunTask(QDateTime lastRun, int id_task)
{
    QSqlQuery query;

    query.prepare("UPDATE dbms_scheduler.jobs SET last_run =:last_run WHERE id=:id");
    query.bindValue(":last_run", lastRun.toString(Qt::ISODateWithMs));
    query.bindValue(":id",id_task);
    query.exec();
    if(query.lastError().isValid())
    {
        qDebug(logWarning())<<__FILE__<<__LINE__<<query.lastError();
        qDebug(logCritical())<<__FILE__<<__LINE__<<query.lastError().nativeErrorCode();
        SystemEventHandler(EventTask::LostConnect);
    }

}

void PGWorker::SystemEventHandler(EventTask event)
{
    switch (event) {
    case EventTask::LostConnect:
    {
        QSqlDatabase db = QSqlDatabase::database();
        if(!db.open())
        {
            qDebug(logInfo()) << "Trying to reconnect...";
            db.close();
            while(!db.open()){
                if (db.open()) {
                    qDebug(logInfo()) << "Reconnected!";
                } else {
                    qDebug(logCritical())<<__FILE__<<__LINE__ << "Could not reconnect to the database. Next try in 2 seconds....";
                }
                QThread::sleep(2);
            }
        }

        break;
    }
    }
}
