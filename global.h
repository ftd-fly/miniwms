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
#include <QGroupBox>
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

#include "task.h"
#include "sql/sql.h"
#include "configure.h"
#include "ui/widgetgood.h"

//全局 变量
extern const QString DATE_TIME_FORMAT;
extern QString g_strExeRoot;
extern Sql *g_sql;
extern Configure configure;

/////////////////////////////////////////////////////////
extern int row;//货物行数
extern int column;//货物列数
extern QList<int> endPoints;//考虑到平移的问题，用于标记该行是否执行到底，
extern QList<WidgetGood *> widgetGoods;//所有的货物

//标记下一个取货A的点
extern int nextTakeColumnA;
extern int nextTakeRowA;

//标记下一个取货B的点
extern int nextTakeRowB;
extern int nextTakeColumnB;
/////////////////////////////////////////////////////////


//全局公用函数
void QyhSleep(int msec);

//随机一个[0,maxRandom)的数
int getRandom(int maxRandom);

int getNextAStation();

int getNextBStation();

#endif // GLOBAL_H
