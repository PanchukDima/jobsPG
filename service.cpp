#include "service.h"
#include <Objects/pgworker.h>
Service::Service(int argc, char **argv) : QtService<QCoreApplication>(argc,argv,"Service")
{
    try{


    //Set service Info

    setServiceDescription("Jobs Postgresql Service.");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}

Service::~Service()
{
    try{

    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}

void Service::start()
{
    try{
        QCoreApplication * app = application();
        PGWorker *pgw = new PGWorker();
        qDebug()<<"Service started";
        pgw->init();
    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}

void Service::pause()
{
    try{

    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}

void Service::resume()
{
    try{

    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}

void Service::stop()
{
    try{

    }
    catch(...)
    {
        qCritical()<<"Error";
    }
}
