#include "global.h"
#include <QTime>
#include <QCoreApplication>

const QString DATE_TIME_FORMAT = "yyyy-MM-dd hh:mm:ss";//统一时间格式
QString g_strExeRoot;
Sql *g_sql = NULL;
moodycamel::ConcurrentQueue<OneLog> g_request_queue;

void QyhSleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);

    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

int getRandom(int maxRandom)
{
    QTime t;
    t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    if(maxRandom>0)
        return qrand()%maxRandom;
    return qrand();
}
