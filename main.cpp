#include "mainwindow.h"
#include <QApplication>
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置全局字体
    a.setFont(QFont("Microsoft Yahei", 14));

    //设置启动图标
    QPixmap pixmap(":/images/logo.png");
    QSplashScreen splash(pixmap);
    splash.show();

    //设置当前目录为根目录
    g_strExeRoot = QCoreApplication::applicationDirPath();
    QDir::setCurrent(g_strExeRoot);

    //载入配置文件
    configure.load();

    //初始化sql
    g_sql = new Sql;
    if(!g_sql->createConnection()){
        QMessageBox::critical(NULL,QStringLiteral("错误"),QStringLiteral("初始化数据库连接失败"),QMessageBox::Ok);
        //return -1;
    }

    //初始化控制中心
    if(!controlCenter.init())
    {
        qDebug() <<"controlCenter init fail";
        //return -1;
    }
    //显示主界面
    MainWindow w;
    splash.finish(&w);
    w.show();

    return a.exec();
}
