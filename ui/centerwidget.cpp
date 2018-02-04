#include <assert.h>
#include "centerwidget.h"
#include "global.h"
#include "widgetgood.h"

CenterWidget::CenterWidget(QWidget *parent) : QWidget(parent)
{
}

void CenterWidget::init()
{
    //载入配置：
    //载入行数列数 货物A的行数
    row = configure.getValue("solution"+solutionPrefix+"/row").toInt();
    column = configure.getValue("solution"+solutionPrefix+"/column").toInt();
    rowA = configure.getValue("solution"+solutionPrefix+"/rowA").toInt();

    //初始化所有的按钮、货物
    for(int j=0;j<row;++j)
    {
        for(int i=0;i<column;++i)
        {
            int type = 1;
            if(j>=rowA)type=2;
            int id = i+j*column;
            if(type==2)id=i+(j-rowA)*column;
            int hasGood = configure.getValue(QString("solution"+solutionPrefix+"/hasGood/%1/%2").arg(j).arg(i)).toInt();
            WidgetGood *good = new WidgetGood(id+1,type,hasGood,solutionPrefix=="B");//方案B中，货物是旋转90°放置的
            widgetGoods.append(good);
        }
    }

    updateNext();

    centergroup = new QGroupBox(QStringLiteral("存放区"));

    //对货物的摆放方式进行设置
    initGoodPosition();

    takeABtn = new QyhClickLabel(QStringLiteral("A空闲"));
    takeBBtn = new QyhClickLabel(QStringLiteral("B空闲"));
    QPushButton *clearBtn = new QPushButton(QStringLiteral("清空所有"));

//    connect(takeABtn,SIGNAL(sigClick()),this,SLOT(onBtnA()));
//    connect(takeBBtn,SIGNAL(sigClick()),this,SLOT(onBtnB()));
    connect(clearBtn,SIGNAL(clicked(bool)),this,SLOT(clear()));

    QHBoxLayout *testTwoBtnHlayout = new QHBoxLayout;
    testTwoBtnHlayout->addStretch(1);
    testTwoBtnHlayout->addWidget(takeABtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(takeBBtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(clearBtn);
    testTwoBtnHlayout->addStretch(1);

    //显示累计值
    QLabel *todayAll = new QLabel(QStringLiteral("今天共计运送:"));
    QLabel *todayA = new QLabel(QStringLiteral("今天共计运送A(个):"));
    QLabel *todayB = new QLabel(QStringLiteral("今天共计运送B(个):"));

    tAll = new QLCDNumber;
    tAll->setDigitCount(5);
    tAll->display(0);
    // 设置显示外观样式
    tAll->setSegmentStyle(QLCDNumber::Flat);
    tAll->setStyleSheet("border: 0px; color: red; background: silver;");
    tAll->setMinimumHeight(40);

    tA = new QLCDNumber;
    tA->setDigitCount(5);
    tA->display(0);
    tA->setSegmentStyle(QLCDNumber::Flat);
    tA->setStyleSheet("border: 0px; color: red; background: silver;");
    tA->setMinimumHeight(40);


    tB = new QLCDNumber;
    tB->setDigitCount(5);
    tB->display(0);
    tB->setSegmentStyle(QLCDNumber::Flat);
    tB->setStyleSheet("border: 0px; color: red; background: silver;");
    tB->setMinimumHeight(40);

    updateNumberTimer.setInterval(5000);
    connect(&updateNumberTimer,SIGNAL(timeout()),this,SLOT(queryNumber()));

    QVBoxLayout *countlayout = new QVBoxLayout;
    countlayout->addStretch(1);
    countlayout->addWidget(todayAll);
    countlayout->addWidget(tAll);
    countlayout->addWidget(todayA);
    countlayout->addWidget(tA);
    countlayout->addWidget(todayB);
    countlayout->addWidget(tB);
    countlayout->addStretch(1);
    countlayout->setMargin(10);
    countlayout->setSpacing(10);


    //水平居中
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(centergroup,2);
    hlayout->addSpacing(10);
    hlayout->addItem(countlayout);

    //竖直居中
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addStretch(1);
    vlayout->addItem(testTwoBtnHlayout);
    vlayout->addItem(hlayout);
    vlayout->addStretch(3);

    setLayout(vlayout);

    updateNumberTimer.start();
}

void CenterWidget::queryNumber()
{
    int aMount = tA->value();
    int bMount = tB->value();
    QString nowTimeStr = QDateTime::currentDateTime().toString(DATE_TIME_FORMAT);
    QString todayStr = nowTimeStr.left(nowTimeStr.indexOf(" "));
    QString fromStr = todayStr + " 00:00:00";
    QString toStr = todayStr + " 23:59:59";

    QString sqlQuery = "select count(id) from agv_task where task_line=? and task_finishTime between ? and ?";
    QStringList params;
    params<<QString("%1").arg((int) Task::LineA)<<fromStr<<toStr;
    QList<QStringList> result = g_sql->query(sqlQuery,params);
    if(result.length()>0&&result.at(0).length()==1)
    {
        aMount = result.at(0).at(0).toInt();
        tA->display(aMount);
    }

    params.clear();
    params<<QString("%1").arg((int) Task::LineB)<<fromStr<<toStr;
    result = g_sql->query(sqlQuery,params);
    if(result.length()>0&&result.at(0).length()==1)
    {
        bMount = result.at(0).at(0).toInt();
        tB->display(bMount);
    }

    tAll->display(aMount+bMount);
}

//取走一个A货物
void CenterWidget::takeGoodA()
{
    if(nextTakeRowA==-1||nextTakeColumnA==-1)
    {
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Yes, this);
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
        mbox.exec();

        return ;
    }

    //判断当前位置是否有货
    if(widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood()<=0)
    {
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Yes, this);
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
        mbox.exec();

        return ;
    }

    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->setHasGood(0);

    //保存配置文件中
    save();

    updateNext();
}

//取走一个B货物
void CenterWidget::takeGoodB()
{
    if(nextTakeRowB==-1||nextTakeColumnB==-1)
    {
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Yes, this);
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
        mbox.exec();
        return ;
    }

    if(widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood()<=0)
    {
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Yes, this);
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
        mbox.exec();
        return ;
    }

    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->setHasGood(0);

    //保存配置文件中
    save();

    updateNext();
}

//保存状态到配置文件
void CenterWidget::save()
{
    //5、每个点位是否有货  hasgood
    for(int i=0;i<row;++i){
        for(int j=0;j<column;++j){
            configure.setValue(QString("solution"+solutionPrefix+"/hasGood/%1/%2").arg(i).arg(j),widgetGoods.at(i*column+j)->hasGood());
        }
    }
    //保存到配置文件
    configure.save();
}


void CenterWidget::onBtnA()
{
    controlCenter.onButtn(RADOI_FREQUENCY_ADDRESS_A);
}

void CenterWidget::onBtnB()
{
    controlCenter.onButtn(RADOI_FREQUENCY_ADDRESS_B);
}

void CenterWidget::clear()
{
    QMessageBox::StandardButton rb = QMessageBox::question(this, QStringLiteral("确认清空"), QStringLiteral("确认清空所有的货物?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确认清空"),QStringLiteral("确认清空所有的货物?"),QMessageBox::Yes | QMessageBox::No, this);
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
        foreach (auto good, widgetGoods) {
            good->setHasGood(0);
        }

        save();

        updateNext();
    }
}


void CenterWidget::updateNext()
{
    minA = 0;
    minB = 0;
    maxA = 0;
    maxB = 0;
    for(int i=0;i<column;++i)
    {
        for(int j=0;j<row;++j){
            int id = widgetGoods.at(j*column+i)->hasGood();
            if(j<rowA){
                if(id>0 && (id<minA || minA ==0)){
                    minA = id;
                    nextTakeColumnA = i;
                    nextTakeRowA = j;
                }
                if(id>maxA){
                    maxA = id;
                }
            }else{
                if(id>0 && (id<minB|| minB ==0)){
                    minB = id;
                    nextTakeColumnB = i;
                    nextTakeRowB = j;
                }
                if(id>maxB){
                    maxB = id;
                }
            }
        }
    }

    foreach (auto p, widgetGoods) {
        p->setTakeFlicker(false);
    }

    if(minA<=0){
        nextTakeColumnA = -1;
        nextTakeRowA = -1;
    }else{
        widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->setTakeFlicker(true);
    }

    if(minB<=0){
        nextTakeColumnB = -1;
        nextTakeRowB = -1;
    }else{
        widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->setTakeFlicker(true);
    }

    //计算takeA的位置
    bool findA = false;
    for(int i=0;i<rowA;++i)
    {
        bool rowHasGood = false;//该行是否有货
        for(int j=0;j<column;++j){
            if(widgetGoods.at(i*column + j)->hasGood()>0){
                rowHasGood = true;
                break;
            }
        }
        if(!rowHasGood)
        {
            //这行没货，那么放货位置就是头部位置
            nextPutColumnA = 0;
            nextPutRowA = i;
            findA = true;
            break;
        }else{
            //找到第一个有货后边无货的位置
            if(widgetGoods.at(i*column+column-1)->hasGood()>0){
                //该行最后一个也是货物，那么这行没法放货物了
                continue;
            }else{
                for(int j=column-2;j>=0;--j){
                    if(widgetGoods.at(i*column + j)->hasGood()>0){
                        //该行最后一个货物的位置是j+1
                        nextPutColumnA = j+1;
                        nextPutRowA = i;
                        findA = true;
                        break;
                    }
                }
                if(findA)break;
            }
        }
    }

    //计算takeB的位置
    bool findB = false;
    for(int i=rowA;i<row;++i)
    {
        bool rowHasGood = false;//该行是否有货
        for(int j=0;j<column;++j){
            if(widgetGoods.at(i*column + j)->hasGood()>0){
                rowHasGood = true;
                break;
            }
        }
        if(!rowHasGood)
        {
            //这行没货，那么放货位置就是头部位置
            nextPutColumnB = 0;
            nextPutRowB = i;
            findB = true;
            break;
        }else{
            //找到第一个有货后边无货的位置
            if(widgetGoods.at(i*column+column-1)->hasGood()>0){
                //该行最后一个也是货物，那么这行没法放货物了
                continue;
            }else{
                for(int j=column-2;j>=0;--j){
                    if(widgetGoods.at(i*column + j)->hasGood()>0){
                        //该行最后一个货物的位置是j+1
                        nextPutColumnB = j+1;
                        nextPutRowB = i;
                        findB = true;
                        break;
                    }
                }
                if(findB)break;
            }
        }
    }
    if(!findA){
        nextPutColumnA = -1;
        nextPutRowA = -1;
    }
    if(!findB){
        nextPutColumnB = -1;
        nextPutRowB = -1;
    }

    for(int i=0;i<row;++i)
    {
        for(int j=0;j<column;++j){
            if( (i==nextPutRowA && j == nextPutColumnA) || (i==nextPutRowB && j == nextPutColumnB))
                widgetGoods.at(i*column+j)->setPutFlicker(true);
            else
                widgetGoods.at(i*column+j)->setPutFlicker(false);
        }
    }

}

int CenterWidget::getNextAStation()
{
    if(nextTakeRowA*column+nextTakeColumnA > widgetGoods.length()||nextTakeRowA*column+nextTakeColumnA<0)return -1;
    if(widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood()>0)
    {
        return  nextTakeRowA*column+nextTakeColumnA;
    }
    return -1;
}

int CenterWidget::getNextBStation()
{
    if(nextTakeRowB*column+nextTakeColumnB > widgetGoods.length()||nextTakeRowB*column+nextTakeColumnB<0)return -1;
    if(widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood()>0)
    {
        return  nextTakeRowB*column+nextTakeColumnB;
    }
    return -1;
}


void CenterWidget::addGood(int add_row, int add_column)
{
    if(add_row<rowA){
        //A
        if(widgetGoods.at(add_row*column+add_column)->hasGood()>0)
        {
            QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("该处已经存在货物"),QMessageBox::Yes, this);
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
            mbox.exec();
        }else{
            widgetGoods.at(add_row*column+add_column)->setHasGood(++maxA);
            save();
            updateNext();
        }
    }else{
        //B
        if(widgetGoods.at(add_row*column+add_column)->hasGood()>0)
        {
            QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("该处已经存在货物"),QMessageBox::Yes, this);
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
            mbox.exec();
        }else{
            widgetGoods.at(add_row*column+add_column)->setHasGood(++maxB);
            save();
            updateNext();
        }
    }
}

void CenterWidget::removeGood(int remove_row,int remove_column)
{
    if(remove_column<0||remove_column>=column ||remove_row<0||remove_row>=row){
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("该处无货物"),QMessageBox::Yes, this);
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
        mbox.exec();
    }else{
        widgetGoods.at(remove_row*column+remove_column)->setHasGood(0);
        save();
        updateNext();
    }
}


void CenterWidget::onStartTakeA()
{
    takeABtn->setText(QStringLiteral("正在取货A中..."));
    takeABtn->setFlicker(true);
}

void CenterWidget::onStartTakeB()
{
    takeBBtn->setText(QStringLiteral("正在取货B中..."));
    takeBBtn->setFlicker(true);
}

void CenterWidget::onFinishTakeA()
{
    takeABtn->setText(QStringLiteral("A空闲"));
    takeABtn->setFlicker(false);
}

void CenterWidget::onFinishTakeB()
{
    takeBBtn->setText(QStringLiteral("B空闲"));
    takeBBtn->setFlicker(false);
}
