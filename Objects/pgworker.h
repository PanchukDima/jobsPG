#ifndef PGWORKER_H
#define PGWORKER_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QDebug>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QThread>
#include "Objects/task.h"
#include <QFile>
#include <QDir>

#include "Objects/logger.h"

struct WorkerObject
{
    QThread *thread;
    Task * task;
};

class PGWorker : public QObject
{
    Q_OBJECT
public:
    explicit PGWorker(QObject *parent = nullptr);
    void init();
    QString getFileConfigPath();
private:

    void getListTasks();
    void startTasks();
    void startTask(int p_id);
    void deploy();
    void deployDB();



    QList<WorkerObject*> workers;

private slots:

    void NotifyHandler(QString, QSqlDriver::NotificationSource,QVariant);
    void UpdateLastRunTask(QDateTime lastRun, int id_task);
    void SystemEventHandler(EventTask);

signals:

};

#endif // PGWORKER_H
