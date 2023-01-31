#include "pgworker.h"

PGWorker::PGWorker(QObject *parent)
    : QObject{parent}
{

}

void PGWorker::init()
{
    deploy();
    QString fileConfig = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[1]+"/connection";
    QSettings settings(fileConfig, QSettings::IniFormat);
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    bool eventNot= db.driver()->hasFeature(QSqlDriver::EventNotifications);
    qDebug(logInfo()) << "Has feature eventNotification" << eventNot;
    db.setHostName(settings.value("host").toString());
    db.setUserName(settings.value("Username").toString());
    db.setPassword(settings.value("password").toString());
    db.setPort(settings.value("port").toInt());
    db.setDatabaseName(settings.value("databasename").toString());
    if(db.open())
    {
        qDebug(logInfo())<<"Connection true";
        QSqlDatabase::database().driver()->subscribeToNotification("dbms_sheduler_jobs");
        connect(QSqlDatabase::database().driver(),SIGNAL(notification(QString,QSqlDriver::NotificationSource,QVariant)),SLOT(NotifyHandler(QString,QSqlDriver::NotificationSource,QVariant)));
        getListTasks();
        startTasks();
    }
    else
    {
        qDebug(logCritical())<<db.lastError();
    }
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
        worker->thread->start();
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

void PGWorker::NotifyHandler(QString val, QSqlDriver::NotificationSource notifySource, QVariant msg)
{
    qDebug(logInfo())<<val<<" "<< notifySource <<" "<< msg;
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
        qDebug(logWarning())<<query.lastError();
    }

}
