#ifndef GLOBAL_H
#define GLOBAL_H

//可能用到的头文件，之后不需要再为头文件操心
#include <QDir>
#include <QFile>
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

//全局 变量
extern const QString DATE_TIME_FORMAT;
extern QString g_strExeRoot;
extern Sql *g_sql;

//呼叫的消息队列
extern moodycamel::ConcurrentQueue<OneLog> g_request_queue;

//全局公用函数
void QyhSleep(int msec);

//随机一个[0,maxRandom)的数
int getRandom(int maxRandom);


#endif // GLOBAL_H
