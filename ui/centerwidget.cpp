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

    nextPutRowA = configure.getValue("solution"+solutionPrefix+"/nextPutRowA").toInt();
    nextPutColumnA = configure.getValue("solution"+solutionPrefix+"/nextPutColumnA").toInt();
    nextPutRowB = configure.getValue("solution"+solutionPrefix+"/nextPutRowB").toInt();
    nextPutColumnB = configure.getValue("solution"+solutionPrefix+"/nextPutColumnB").toInt();

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
    oldtakeABtn = new QPushButton(QStringLiteral("A取货"));
    oldtakeBBtn = new QPushButton(QStringLiteral("B取货"));
    takeABtn = new QyhClickLabel(QStringLiteral("A空闲"));
    takeBBtn = new QyhClickLabel(QStringLiteral("B空闲"));
    QPushButton *clearBtn = new QPushButton(QStringLiteral("清空所有"));
    cancelABtn = new QPushButton(QStringLiteral("取消A任务"));
    cancelBBtn = new QPushButton(QStringLiteral("取消B任务"));

    connect(cancelABtn,SIGNAL(clicked(bool)),this,SLOT(cancelA()));
    connect(cancelBBtn,SIGNAL(clicked(bool)),this,SLOT(cancelB()));

    connect(oldtakeABtn,SIGNAL(clicked(bool)),this,SLOT(takeA()));
    connect(oldtakeBBtn,SIGNAL(clicked(bool)),this,SLOT(takeB()));

    connect(clearBtn,SIGNAL(clicked(bool)),this,SLOT(clear()));

    cancelABtn->setEnabled(false);
    cancelBBtn->setEnabled(false);

    QHBoxLayout *testTwoBtnHlayout = new QHBoxLayout;
    testTwoBtnHlayout->addStretch(1);
    testTwoBtnHlayout->addWidget(takeABtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(takeBBtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(clearBtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(cancelABtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(cancelBBtn);

    //这两个按钮仅用于调试
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(oldtakeABtn);
    testTwoBtnHlayout->addSpacing(100);
    testTwoBtnHlayout->addWidget(oldtakeBBtn);

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
    hlayout->addStretch(1);
    hlayout->addWidget(centergroup,2);
    hlayout->addSpacing(10);
    hlayout->addItem(countlayout);
    hlayout->addStretch(1);

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

    onStartTakeA();

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

    onStartTakeB();

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

    configure.setValue(QString("solution"+solutionPrefix+"/nextPutRowA"),nextPutRowA);
    configure.setValue(QString("solution"+solutionPrefix+"/nextPutRowB"),nextPutRowB);
    configure.setValue(QString("solution"+solutionPrefix+"/nextPutColumnA"),nextPutColumnA);
    configure.setValue(QString("solution"+solutionPrefix+"/nextPutColumnB"),nextPutColumnB);

    //保存到配置文件
    configure.save();
}


void CenterWidget::clear()
{
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

        nextPutRowA = 0;
        nextPutColumnA = 0;
        nextPutRowB = rowA;
        nextPutColumnB = 0;

        save();

        updateNext();
    }
}

void CenterWidget::cancelA()
{
    QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确认取消"),QStringLiteral("确认取消正在执行的A任务?"),QMessageBox::Yes | QMessageBox::No, this);
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
        controlCenter.cancelTask(RADOI_FREQUENCY_ADDRESS_A);

        QMessageBox mbox2(QMessageBox::NoIcon,QStringLiteral("确认状态"),QStringLiteral("刚取消了正在执行的任务A，待状态可以继续执行后续任务后，请点击确认?"),QMessageBox::Yes, this);
        mbox2.setStyleSheet(
                    "QPushButton {"
                    "font:30px;"
                    "padding-left:100px;"
                    "padding-right:100px;"
                    "padding-top:40px;"
                    "padding-bottom:40px;"
                    "}"
                    "QLabel { font:30px;}"
                    );
        mbox2.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));

        int rr = mbox2.exec();
        if(rr==QMessageBox::Yes)
        {
            controlCenter.recover();
        }
    }
}

void CenterWidget::cancelB()
{
    QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确认取消"),QStringLiteral("确认取消正在执行的B任务?"),QMessageBox::Yes | QMessageBox::No, this);
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
        controlCenter.cancelTask(RADOI_FREQUENCY_ADDRESS_B);

        QMessageBox mbox2(QMessageBox::NoIcon,QStringLiteral("确认状态"),QStringLiteral("刚取消了任务，待状态可以继续执行后续任务后，请点击确认?"),QMessageBox::Yes, this);
        mbox2.setStyleSheet(
                    "QPushButton {"
                    "font:30px;"
                    "padding-left:100px;"
                    "padding-right:100px;"
                    "padding-top:40px;"
                    "padding-bottom:40px;"
                    "}"
                    "QLabel { font:30px;}"
                    );
        mbox2.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));

        int rr = mbox2.exec();
        if(rr==QMessageBox::Yes)
        {
            controlCenter.recover();
        }
    }
}

void CenterWidget::finishA()
{
    onFinishTakeA();
}

void CenterWidget::finishB()
{
    onFinishTakeB();
}

void CenterWidget::updateNextPutA()
{
    //发生了取货、放货、删除货物三种操作
    //1.判断放货
    if(nextPutRowA>=0 && nextPutColumnA>=0 && widgetGoods.at(nextPutRowA*column+nextPutColumnA)->hasGood()>0)
    {
        //放货了，往下移
        nextPutColumnA += 1;
        if(nextPutColumnA == column){
            //这行满了，那么就另寻一行
            int tempRow = -1,tempColumn = -1;
            for(int k=0;k<rowA;++k)
            {
                for(int m=column-1;m>=0;--m)
                {
                    if(widgetGoods.at(k*column+m)->hasGood()>0){
                        break;
                    }else{
                        tempRow=k;
                        tempColumn=m;
                    }
                }
                if(tempRow>=0&&tempColumn>=0)break;
            }
            //找到了
            if(tempRow>=0&&tempColumn>=0){
                nextPutColumnA = tempColumn;
                nextPutRowA = tempRow;
            }else{
                nextPutRowA = 0;
                nextPutColumnA = -1;
            }
        }
    }
    //2.判断取货



    //3.删除货物

}

void CenterWidget::updateNextPutB()
{

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
        p->setStatus(p->hasGood()>0?WidgetGood::GOOD_STATUS_YES:WidgetGood::GOOD_STATUS_NO);
    }

    if(minA<=0){
        nextTakeColumnA = -1;
        nextTakeRowA = -1;
    }else{
        widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->setStatus(WidgetGood::GOOD_STATUS_TOTAKE);
    }

    if(minB<=0){
        nextTakeColumnB = -1;
        nextTakeRowB = -1;
    }else{
        widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->setStatus(WidgetGood::GOOD_STATUS_TOTAKE);
    }



    //    //计算putA的位置
    //    bool findA = false;
    //    for(int i=0;i<rowA;++i)
    //    {
    //        bool rowHasGood = false;//该行是否有货
    //        for(int j=0;j<column;++j){
    //            if(widgetGoods.at(i*column + j)->hasGood()>0){
    //                rowHasGood = true;
    //                break;
    //            }
    //        }
    //        if(!rowHasGood)
    //        {
    //            //这行没货，那么放货位置就是头部位置
    //            nextPutColumnA = 0;
    //            nextPutRowA = i;
    //            findA = true;
    //            break;
    //        }else{
    //            //找到第一个有货后边无货的位置
    //            if(widgetGoods.at(i*column+column-1)->hasGood()>0){
    //                //该行最后一个也是货物，那么这行没法放货物了
    //                continue;
    //            }else{
    //                for(int j=column-2;j>=0;--j){
    //                    if(widgetGoods.at(i*column + j)->hasGood()>0){
    //                        //该行最后一个货物的位置是j+1
    //                        nextPutColumnA = j+1;
    //                        nextPutRowA = i;
    //                        findA = true;
    //                        break;
    //                    }
    //                }
    //                if(findA)break;
    //            }
    //        }
    //    }

    //    //计算putB的位置
    //    bool findB = false;
    //    for(int i=rowA;i<row;++i)
    //    {
    //        bool rowHasGood = false;//该行是否有货
    //        for(int j=0;j<column;++j){
    //            if(widgetGoods.at(i*column + j)->hasGood()>0){
    //                rowHasGood = true;
    //                break;
    //            }
    //        }
    //        if(!rowHasGood)
    //        {
    //            //这行没货，那么放货位置就是头部位置
    //            nextPutColumnB = 0;
    //            nextPutRowB = i;
    //            findB = true;
    //            break;
    //        }else{
    //            //找到第一个有货后边无货的位置
    //            if(widgetGoods.at(i*column+column-1)->hasGood()>0){
    //                //该行最后一个也是货物，那么这行没法放货物了
    //                continue;
    //            }else{
    //                for(int j=column-2;j>=0;--j){
    //                    if(widgetGoods.at(i*column + j)->hasGood()>0){
    //                        //该行最后一个货物的位置是j+1
    //                        nextPutColumnB = j+1;
    //                        nextPutRowB = i;
    //                        findB = true;
    //                        break;
    //                    }
    //                }
    //                if(findB)break;
    //            }
    //        }
    //    }
    //    if(!findA){
    //        nextPutColumnA = -1;
    //        nextPutRowA = -1;
    //    }
    //    if(!findB){
    //        nextPutColumnB = -1;
    //        nextPutRowB = -1;
    //    }

    for(int i=0;i<row;++i)
    {
        for(int j=0;j<column;++j){
            if( (i==nextPutRowA && j == nextPutColumnA) || (i==nextPutRowB && j == nextPutColumnB))
                widgetGoods.at(i*column+j)->setStatus(WidgetGood::GOOD_STATUS_TOPUT);
        }
    }

    for(int i=0;i<row;++i)
    {
        for(int j=0;j<column;++j){
            if( (i==takingRowA && j == takingColumnA) || (i==takingRowB && j == takingColumnB))
                widgetGoods.at(i*column+j)->setStatus(WidgetGood::GOOD_STATUS_TAKING);
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

            assert(add_row == nextPutRowA && add_column == nextPutColumnA);
            //放货了，往下移
            nextPutColumnA += 1;
            if(nextPutColumnA == column){
                //这行满了，那么就另寻一行
                int tempRow = -1,tempColumn = -1;
                for(int k=0;k<rowA;++k)
                {
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at(k*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempRow=k;
                            tempColumn=m;
                        }
                    }
                    if(tempRow>=0&&tempColumn>=0)break;
                }
                //找到了
                if(tempRow>=0&&tempColumn>=0){
                    nextPutColumnA = tempColumn;
                    nextPutRowA = tempRow;
                }else{
                    nextPutRowA = 0;
                    nextPutColumnA = -1;
                }
            }

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

            assert(add_row == nextPutRowB && add_column == nextPutColumnB);

            nextPutColumnB += 1;
            if(nextPutColumnB == column)
            {
                //找到第一个有货后边无货的位置
                int tempRow = -1,tempColumn = -1;
                for(int k=rowA;k<row;++k)
                {
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at(k*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempRow=k;
                            tempColumn=m;
                        }
                    }
                    if(tempRow>=0&&tempColumn>=0)break;
                }
                //找到了
                if(tempRow>=0&&tempColumn>=0){
                    nextPutColumnB = tempColumn;
                    nextPutRowB = tempRow;
                }else{
                    nextPutRowB = rowA;
                    nextPutColumnB = -1;
                }
            }

            save();
            updateNext();
        }
    }
}

void CenterWidget::takeA()
{
    controlCenter.onButtn(RADOI_FREQUENCY_ADDRESS_A);
}

void CenterWidget::takeB()
{
    controlCenter.onButtn(RADOI_FREQUENCY_ADDRESS_B);
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

        if(remove_row<rowA)
        {
            //A
            if((remove_row == nextPutRowA && remove_column == nextPutColumnA - 1 )|| (nextPutRowA ==0 && nextPutColumnA == -1))
            {
                int tempColumn = -1;
                for(int m=column-1;m>=0;--m)
                {
                    int status = widgetGoods.at(remove_row*column+m)->getStatus();
                    if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                        break;
                    }else{
                        tempColumn=m;
                    }
                }
                if(tempColumn >= 0)
                {
                    nextPutRowA = remove_row;
                    nextPutColumnA = tempColumn;
                }

                while(true){
                    if(nextPutColumnA == 0 && nextPutRowA >0)
                    {
                        //上一行是否有空位置
                        tempColumn = -1;
                        for(int m=column-1;m>=0;--m)
                        {
                            int status = widgetGoods.at((nextPutRowA - 1)*column+m)->getStatus();
                            if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                                break;
                            }else{
                                tempColumn=m;
                            }
                        }
                        if(tempColumn >= 0)
                        {
                            nextPutRowA = nextPutRowA - 1;
                            nextPutColumnA = tempColumn;
                        }else{
                            break;
                        }
                    }else{
                        break;
                    }
                }
            }else if(remove_row +1 == nextPutRowA && nextPutColumnA == 0)
            {
                //找上一行的最后的空位位置
                int tempColumn = -1;
                for(int m=column-1;m>=0;--m)
                {
                    int status = widgetGoods.at(remove_row*column+m)->getStatus();
                    if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                        break;
                    }else{
                        tempColumn=m;
                        break;
                    }
                }

                //找到了
                if(tempColumn>=0){
                    nextPutColumnA = tempColumn;
                    nextPutRowA = remove_row;
                }

                while(true){
                    if(nextPutColumnA == 0 && nextPutRowA >0)
                    {
                        //上一行是否有空位置
                        tempColumn = -1;
                        for(int m=column-1;m>=0;--m)
                        {
                            int status = widgetGoods.at((nextPutRowA - 1)*column+m)->getStatus();
                            if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                                break;
                            }else{
                                tempColumn=m;
                            }
                        }
                        if(tempColumn >= 0)
                        {
                            nextPutRowA = nextPutRowA - 1;
                            nextPutColumnA = tempColumn;
                        }else{
                            break;
                        }
                    }else{
                        break;
                    }
                }
            }
        }else{
            //B
            if((remove_row == nextPutRowB && remove_column == nextPutColumnB - 1 )|| (nextPutRowB == rowA && nextPutColumnB == -1))
            {
                int tempColumn = -1;
                for(int m=column-1;m>=0;--m)
                {
                    int status = widgetGoods.at(remove_row*column+m)->getStatus();
                    if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                        break;
                    }else{
                        tempColumn=m;
                    }
                }
                if(tempColumn >= 0)
                {
                    nextPutRowB = remove_row;
                    nextPutColumnB = tempColumn;
                }

                while(true){
                    if(nextPutColumnB == 0 && nextPutRowB > rowA)
                    {
                        //上一行是否有空位置
                        tempColumn = -1;
                        for(int m=column-1;m>=0;--m)
                        {
                            int status = widgetGoods.at((nextPutRowB - 1)*column+m)->getStatus();
                            if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                                break;
                            }else{
                                tempColumn=m;
                            }
                        }
                        if(tempColumn >= 0)
                        {
                            nextPutRowB = nextPutRowB - 1;
                            nextPutColumnB = tempColumn;
                        }else{
                            break;
                        }
                    }else{
                        break;
                    }
                }
            }else if(remove_row +1 == nextPutRowB && nextPutColumnB == 0)
            {
                //找上一行的最后的空位位置
                int tempColumn = -1;
                for(int m=column-1;m>=0;--m)
                {
                    int status = widgetGoods.at(remove_row*column+m)->getStatus();
                    if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                        break;
                    }else{
                        tempColumn=m;
                        break;
                    }
                }

                //找到了
                if(tempColumn>=0){
                    nextPutColumnB = tempColumn;
                    nextPutRowB = remove_row;
                }

                while(true){
                    if(nextPutColumnB == 0 && nextPutRowB > rowA)
                    {
                        //上一行是否有空位置
                        tempColumn = -1;
                        for(int m=column-1;m>=0;--m)
                        {
                            int status = widgetGoods.at((nextPutRowB-1)*column+m)->getStatus();
                            if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                                break;
                            }else{
                                tempColumn=m;
                            }
                        }
                        if(tempColumn >= 0)
                        {
                            nextPutRowB = nextPutRowB - 1;
                            nextPutColumnB = tempColumn;
                        }else{
                            break;
                        }
                    }else{
                        break;
                    }
                }
            }
        }

        save();
        updateNext();
    }
}


void CenterWidget::onStartTakeA()
{
    takeABtn->setText(QStringLiteral("正在取货A中..."));
    takeABtn->setFlicker(true);
    cancelABtn->setEnabled(true);
    takingRowA = nextTakeRowA;
    takingColumnA = nextTakeColumnA;
}

void CenterWidget::onStartTakeB()
{
    takeBBtn->setText(QStringLiteral("正在取货B中..."));
    takeBBtn->setFlicker(true);
    cancelBBtn->setEnabled(true);
    takingRowB = nextTakeRowB;
    takingColumnB = nextTakeColumnB;
}

void CenterWidget::onFinishTakeA()
{
    takeABtn->setText(QStringLiteral("A空闲"));
    takeABtn->setFlicker(false);
    cancelABtn->setEnabled(false);

    if(takingColumnA>=0 && takingRowA >=0)
    {
        int remove_row = takingRowA;
        int remove_column = takingColumnA;

        widgetGoods.at(remove_row*column+remove_column)->setHasGood(0);

        //A
        if((remove_row == nextPutRowA && remove_column == nextPutColumnA - 1 )|| (nextPutRowA ==0 && nextPutColumnA == -1))
        {
            int tempColumn = -1;
            for(int m=column-1;m>=0;--m)
            {
                int status = widgetGoods.at(remove_row*column+m)->getStatus();
                if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                    break;
                }else{
                    tempColumn=m;
                }
            }
            if(tempColumn >= 0)
            {
                nextPutRowA = remove_row;
                nextPutColumnA = tempColumn;
            }

            while(true){
                if(nextPutColumnA == 0 && nextPutRowA >0)
                {
                    //上一行是否有空位置
                    tempColumn = -1;
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at((nextPutRowA - 1)*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempColumn=m;
                        }
                    }
                    if(tempColumn >= 0)
                    {
                        nextPutRowA = nextPutRowA - 1;
                        nextPutColumnA = tempColumn;
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }
        }else if(remove_row +1 == nextPutRowA && nextPutColumnA == 0)
        {
            //找上一行的最后的空位位置
            int tempColumn = -1;
            for(int m=column-1;m>=0;--m)
            {
                int status = widgetGoods.at(remove_row*column+m)->getStatus();
                if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                    break;
                }else{
                    tempColumn=m;
                    break;
                }
            }

            //找到了
            if(tempColumn>=0){
                nextPutColumnA = tempColumn;
                nextPutRowA = remove_row;
            }

            while(true){
                if(nextPutColumnA == 0 && nextPutRowA >0)
                {
                    //上一行是否有空位置
                    tempColumn = -1;
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at((nextPutRowA - 1)*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempColumn=m;
                        }
                    }
                    if(tempColumn >= 0)
                    {
                        nextPutRowA = nextPutRowA - 1;
                        nextPutColumnA = tempColumn;
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }
        }
    }

    takingRowA = -1;
    takingColumnA = -1;

    updateNext();
}

void CenterWidget::onFinishTakeB()
{
    takeBBtn->setText(QStringLiteral("B空闲"));
    takeBBtn->setFlicker(false);
    cancelBBtn->setEnabled(false);

    if(takingColumnB>=0 && takingRowB >=0)
    {
        int remove_row = takingRowB;
        int remove_column = takingColumnB;

        widgetGoods.at(remove_row*column+remove_column)->setHasGood(0);

        if((remove_row == nextPutRowB && remove_column == nextPutColumnB - 1 )|| (nextPutRowB == rowA && nextPutColumnB == -1))
        {
            int tempColumn = -1;
            for(int m=column-1;m>=0;--m)
            {
                int status = widgetGoods.at(remove_row*column+m)->getStatus();
                if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                    break;
                }else{
                    tempColumn=m;
                }
            }
            if(tempColumn >= 0)
            {
                nextPutRowB = remove_row;
                nextPutColumnB = tempColumn;
            }

            while(true){
                if(nextPutColumnB == 0 && nextPutRowB > rowA)
                {
                    //上一行是否有空位置
                    tempColumn = -1;
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at(nextPutRowB*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempColumn=m;
                        }
                    }
                    if(tempColumn >= 0)
                    {
                        nextPutRowB = nextPutRowB - 1;
                        nextPutColumnB = tempColumn;
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }
        }else if(remove_row +1 == nextPutRowB && nextPutColumnB == 0)
        {
            //找上一行的最后的空位位置
            int tempColumn = -1;
            for(int m=column-1;m>=0;--m)
            {
                int status = widgetGoods.at(remove_row*column+m)->getStatus();
                if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                    break;
                }else{
                    tempColumn=m;
                    break;
                }
            }

            //找到了
            if(tempColumn>=0){
                nextPutColumnB = tempColumn;
                nextPutRowB = remove_row;
            }

            while(true){
                if(nextPutColumnB == 0 && nextPutRowB > rowA)
                {
                    //上一行是否有空位置
                    tempColumn = -1;
                    for(int m=column-1;m>=0;--m)
                    {
                        int status = widgetGoods.at((nextPutRowB - 1)*column+m)->getStatus();
                        if(status != WidgetGood::GOOD_STATUS_NO && status != WidgetGood::GOOD_STATUS_TOPUT){
                            break;
                        }else{
                            tempColumn=m;
                        }
                    }
                    if(tempColumn >= 0)
                    {
                        nextPutRowB = nextPutRowB - 1;
                        nextPutColumnB = tempColumn;
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }
        }
    }

    takingRowB = -1;
    takingColumnB = -1;

    updateNext();
}
