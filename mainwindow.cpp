#include "mainwindow.h"
#include "global.h"

#include "ui/widgettypea.h"
#include "ui/widgettypeb.h"
#include "ui/widgettypec.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //设置默认的尺寸为当前屏幕分辨率
    this->setMinimumSize(1560,900);
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
    QMessageBox::StandardButton rb = QMessageBox::question(this, QStringLiteral("确认退出"), QStringLiteral("确认退出吗?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes)
    {
        event->accept();
    }else{
        event->ignore();
    }
}
