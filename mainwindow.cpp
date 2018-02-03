#include "mainwindow.h"
#include "global.h"

#include "ui/widgettypea.h"
#include "ui/widgettypeb.h"
#include "ui/widgettypec.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //设置默认的尺寸为当前屏幕分辨率
    this->setMinimumSize(1920,1080);
    //载入CentralWidget
    if(configure.getValue("solution/type").toString() == "A")
    {
        solutionPrefix = "A";
        centerWidget = new WidgetTypeA;
    }else if(configure.getValue("solution/type").toString() == "B")
    {
        solutionPrefix = "B";
        centerWidget = new WidgetTypeB;
    }else{
        solutionPrefix = "C";
        centerWidget = new WidgetTypeC;
    }
    setCentralWidget(centerWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确认退出"),QStringLiteral("确认退出吗?"),QMessageBox::Yes | QMessageBox::No, this);
    mbox.setStyleSheet(
                "QPushButton {"
                "font:30px;"
                "padding-left:100px;"
                "padding-right:100px;"
                "padding-top:40px;"
                "padding-bottom:40px;"
                "}"
                "QLabel { font:30px;}"
                );
    mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
    mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));

    int r = mbox.exec();
    if(r==QMessageBox::Yes)
    {
        event->accept();
    }else{
        event->ignore();
    }
}
