#ifndef GLOBAL_H
#define GLOBAL_H

//可能用到的头文件，之后不需要再为头文件操心
#include <QDir>
#include <QFile>
#include <QSplashScreen>
#include <QApplication>
#include <QVariant>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QTableWidget>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTabWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QRegExpValidator>
#include <QRegExp>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QColor>
#include <QGraphicsItem>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QWidget>
#include <QDebug>

#include "concurrentqueue.h"
#include "sql/sql.h"
#include "configure.h"

//全局 变量
extern const QString DATE_TIME_FORMAT;
extern QString g_strExeRoot;
extern Sql *g_sql;
extern Configure configure;

//呼叫送货消息[保存于内存中 g_request_queue]
struct RequestGoodMsg
{
public:
    RequestGoodMsg()
    {}
    int type;//站点呼叫的货物类型
    int station;//呼叫站点
    QDateTime time;//呼叫时间

    RequestGoodMsg(const RequestGoodMsg& b){
        type = b.type;
        station = b.station;
        if(b.time.isValid()){
            time = b.time;
        }
    }
    bool operator <(const RequestGoodMsg &b){
        return time<b.time;
    }
};

//送货消息,于请求相对应的[保存于数据库中]
struct ResponseGoodMsg
{
public:
    ResponseGoodMsg() {}
    RequestGoodMsg request;
    int id;//这一个送货单的id
    int agvId;//送货执行车辆ID
    QDateTime excuteTime;//执行时间，理论上车辆不忙，没有任务排队的话是和请求时间一样的
    QDateTime finishTime;//完成时间，货物送完后的这个时间点

    ResponseGoodMsg(const ResponseGoodMsg& b){
        id = b.id;
        agvId = b.agvId;
        if(b.excuteTime.isValid()){
            excuteTime = b.excuteTime;
        }
        if(b.finishTime.isValid()){
            finishTime = b.finishTime;
        }
        request = b.request;
    }
    bool operator <(const ResponseGoodMsg &b){
        return request.time<b.request.time;
    }
};


//呼叫的消息队列
extern moodycamel::ConcurrentQueue<RequestGoodMsg> g_request_queue;

//全局公用函数
void QyhSleep(int msec);

//随机一个[0,maxRandom)的数
int getRandom(int maxRandom);


#endif // GLOBAL_H
