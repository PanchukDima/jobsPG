#ifndef SERVICE_H
#define SERVICE_H

#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include <qtservice/src/qtservice.h>

class Service : public QtService<QCoreApplication>
{
public:
    /**
     * @brief The constructor
     * @param argc
     * @param argv
     */
    Service(int argc, char **argv);
    /**
     * @brief The destructor
     */
    ~Service();
    /**
     * @brief start Service
     */
    void start();

    /**
     * @brief pause Service
     */

    void pause();

    /**
     * @brief resume Service
     */

    void resume();

    /**
     * @brief stop Service
     */

    void stop();
private:


};

#endif // SERVICE_H
