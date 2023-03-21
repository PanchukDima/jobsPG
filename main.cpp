#include <QCoreApplication>
#include <Objects/pgworker.h>
#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>
#include <QStandardPaths>

// Умный указатель на файл логирования
QScopedPointer<QFile>   m_logFile;

// Объявляение обработчика
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString logFilePath;
    QString logFileName = "PGworker.log";
    #ifdef __linux__
        logFilePath = "/var/log/jobsPG/";
    #elif _WIN32
        logFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    #endif
    QDir log(logFilePath);
    if(!log.exists())
    {
        log.mkdir(logFilePath);
    }
    m_logFile.reset(new QFile(logFilePath+logFileName));

    m_logFile.data()->open(QFile::Append | QFile::Text);
    //qInstallMessageHandler(messageHandler);
    PGWorker *pgw = new PGWorker();
    pgw->init();

    return a.exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Открываем поток записи в файл
    QTextStream out(m_logFile.data());
    // Записываем дату записи
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
    // Записываем в вывод категорию сообщения и само сообщение
    out << context.category << ": "<< msg <<"\n";
    out.flush();    // Очищаем буферизированные данные
}
