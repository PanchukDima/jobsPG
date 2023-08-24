#include <QCoreApplication>
#include <Objects/pgworker.h>
#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QSettings>
#include <service.h>

// Умный указатель на файл логирования
QScopedPointer<QFile>   m_logFile;
QString loggerLevel;

QString version = "0.6.0";

// Объявляение обработчика
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    #ifdef __linux__
         QCoreApplication a(argc, argv);
    #elif _WIN32
        Service a(argc, argv);
    #endif


    QString logFilePath;
    QString logFileName = "PGworker"+QDate::currentDate().toString("MM_yyyy")+".log";
    #ifdef __linux__
        logFilePath = "/var/log/jobsPG/";
    #elif _WIN32
        logFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/";
    #endif
    QDir log(logFilePath);
    if(!log.exists())
    {
        log.mkdir(logFilePath);
    }
    m_logFile.reset(new QFile(logFilePath+logFileName));
    qDebug()<<logFilePath+logFileName;
    m_logFile.data()->open(QFile::Append | QFile::Text);
    PGWorker *pgw = new PGWorker();
    QString configPath = pgw->getFileConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);
    if(settings.value("logLevel").toString() != "")
    {
        loggerLevel = settings.value("logLevel").toString();
    }
    else
    {
        loggerLevel = "INF";
    }
    #ifdef __linux__
        if (a.arguments().count() > 1){
            if (a.arguments().at(1) != "console")
            {
                qDebug()<<"No console";

            }
        }
        else
        {
            qInstallMessageHandler(messageHandler);
        }
    #endif

    #ifdef __linux__
        pgw->init();
    #endif

    return a.exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Открываем поток записи в файл

    QTextStream out(m_logFile.data());
    // Записываем дату записи
    bool iswrite = false;
    if(loggerLevel == "INF" && context.category == "Info")
    {
       iswrite = true;
    }
    if(loggerLevel == "DBG")
    {
        iswrite = true;
    }
    if(iswrite){
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
        // По типу определяем, к какому уровню относится сообщение
        switch (type)
        {
        case QtInfoMsg:     out << "INF "; break;
        case QtDebugMsg:    out << "DBG "; break;
        case QtWarningMsg:  out << "WRN "; break;
        case QtCriticalMsg: out << "CRT "; break;
        case QtFatalMsg:    out << "FTL "; break;
        }

        out << context.category <<":"<<"Ver."<<version <<":"<< ": "<< msg <<"\n";
        out.flush();
    }
    //if(loggerLevel == "DBG")
//    {
//        out << context.category <<":"<<"Ver."<<version <<":"<< ": "<< msg <<"\n";
//    }
      // Очищаем буферизированные данные
}
