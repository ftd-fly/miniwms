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

    //载入每行的endpoint
    endPoints.clear();
    for(int i=0;i<row;++i){
        endPoints.push_back(configure.getValue(QString("solution"+solutionPrefix+"/endpoint/%1").arg(i)).toInt());
    }

    //载入当前取货行和列
    nextTakeColumnA = configure.getValue("solution"+solutionPrefix+"/nextTakeColumnA").toInt();
    nextTakeRowA = configure.getValue("solution"+solutionPrefix+"/nextTakeRowA").toInt();
    nextTakeColumnB = configure.getValue("solution"+solutionPrefix+"/nextTakeColumnB").toInt();
    nextTakeRowB = configure.getValue("solution"+solutionPrefix+"/nextTakeRowB").toInt();

    //初始化所有的按钮、货物
    for(int j=0;j<row;++j)
    {
        //添加两个按钮
        QPushButton *traBtn = new QPushButton(QStringLiteral("该行货物平移→"),this);
        QSizePolicy sp_retain = traBtn->sizePolicy();
        sp_retain.setRetainSizeWhenHidden(true);
        traBtn->setSizePolicy(sp_retain);
        translationButtons.append(traBtn);
        connect(traBtn,SIGNAL(clicked(bool)),this,SLOT(translation()));

        QPushButton *fillBtn = new QPushButton(QStringLiteral("该行货物已补满"),this);
        QSizePolicy sp_retain2 = fillBtn->sizePolicy();
        sp_retain2.setRetainSizeWhenHidden(true);
        fillBtn->setSizePolicy(sp_retain2);
        fillButtons.append(fillBtn);
        connect(fillBtn,SIGNAL(clicked(bool)),this,SLOT(fillRow()));

        for(int i=0;i<column;++i)
        {
            int type = 1;
            if(j>=rowA)type=2;
            int id = i+j*column;
            if(type==2)id=i+(j-rowA)*column;
            bool hasGood = configure.getValue(QString("solution"+solutionPrefix+"/hasGood/%1/%2").arg(j).arg(i)).toBool();
            WidgetGood *good = new WidgetGood(id+1,type,hasGood,solutionPrefix=="B");//方案B中，货物是旋转90°放置的
            widgetGoods.append(good);
        }
    }

    centergroup = new QGroupBox(QStringLiteral("存放区"));

    //对货物的摆放方式进行设置
    initGoodPosition();

    QPushButton *takeABtn = new QPushButton(QStringLiteral("取走一个A"));
    QPushButton *takeBBtn = new QPushButton(QStringLiteral("取走一个B"));

    connect(takeABtn,SIGNAL(clicked(bool)),this,SLOT(takeGoodA()));
    connect(takeBBtn,SIGNAL(clicked(bool)),this,SLOT(takeGoodB()));

    QHBoxLayout *testTwoBtnHlayout = new QHBoxLayout;
    testTwoBtnHlayout->addStretch(1);
    testTwoBtnHlayout->addWidget(takeABtn);
    testTwoBtnHlayout->addStretch(1);
    testTwoBtnHlayout->addWidget(takeBBtn);
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
    countlayout->setMargin(40);
    countlayout->setSpacing(10);


    //水平居中
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addWidget(centergroup);
    hlayout->addItem(countlayout);
    hlayout->addStretch(1);

    //竖直居中
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addStretch(1);
    vlayout->addItem(testTwoBtnHlayout);
    vlayout->addItem(hlayout);
    vlayout->addStretch(1);

    setLayout(vlayout);
    updateBtnsFlickers();

    updateNumberTimer.start();
}

//填满一行货物
void CenterWidget::fillRow()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    int index = -1 ;
    for(int i=0;i<fillButtons.length();++i ){
        if(fillButtons.at(i) == clickedButton){
            index = i;
            break;
        }
    }

    assert(index != -1);
    //检查该行是否为空
    for(int i=column*index;i<column*(index+1);++i)
    {
        if(widgetGoods.at(i)->hasGood()){
            //这行还有货物
            QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("该行尚有余货！"),QMessageBox::Ok);
            return ;
        }
    }

    QMessageBox::StandardButton rb = QMessageBox::question(this,QStringLiteral("确认"),QStringLiteral("确认该行已填满？"),QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes)
    {
        for(int i=column*index;i<column*(index+1);++i)
        {
            widgetGoods.at(i)->setHasGood(true);
        }
        save();
        updateBtnsFlickers();
    }
}

//平移一行货物
void CenterWidget::translation()
{
    //判断该行货物补满！
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    int index = -1 ;
    for(int i=0;i<translationButtons.length();++i ){
        if(translationButtons.at(i) == clickedButton){
            index = i;
            break;
        }
    }
    assert(index != -1);

    //检查该行是否为空
    int goodAmount = 0;
    for(int i=column*index;i<column*(index+1);++i)
    {
        if(widgetGoods.at(i)->hasGood())++goodAmount;
    }
    if(goodAmount==0)
    {
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("该行没有货物可移动"),QMessageBox::Ok);
        return ;
    }
    if(goodAmount==column){
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("该行货物是满的，无法移动"),QMessageBox::Ok);
        return ;
    }

    QMessageBox::StandardButton rb = QMessageBox::question(this,QStringLiteral("确认"),QStringLiteral("确认该行原来货物已平移并将后面空余填充？"),QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes)
    {
        //OK接下来的逻辑就有一点点意思了
        //给这行一个特殊的标记(endPoint);理论上该行的endpoint都是0；如果平移了，endpoint不再是0。
        endPoints[index] = column - goodAmount;
        //该行货物补齐
        for(int i=column*index;i<column*(index+1);++i)
        {
            widgetGoods.at(i)->setHasGood(true);
        }
        save();
        //更新当前取货位置
        if(index<rowA)
        {
            nextTakeColumnA = 0;
        }else{
            nextTakeColumnB = 0;
        }

        updateBtnsFlickers();
    }
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
    //判断当前位置是否有货
    if(!widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood())
    {
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Ok);
        return ;
    }
    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->setHasGood(false);
    nextTakeColumnA+=1;

    if(nextTakeColumnA >= column -  endPoints.at(nextTakeRowA))
    {
        if(endPoints.at(nextTakeRowA) !=0){
            endPoints[nextTakeRowA] = 0;
        }
        //指向下一行首个位置取货
        nextTakeRowA+=1;
        nextTakeColumnA = 0;
        if(nextTakeRowA>=rowA){
            nextTakeRowA = 0;
        }

        if(!widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood())
        {
            //如果首个位置没有货物，判断是否该行其他位置有货，指向该行剩余的
            for(int columnIndex = nextTakeColumnA;columnIndex< column-endPoints.at(nextTakeRowA);++columnIndex)
            {
                if(widgetGoods.at(nextTakeRowA*column+columnIndex)->hasGood()){
                    nextTakeColumnA = columnIndex;
                    break;
                }
            }
        }
    }
    save();
    //对按钮状态进行设置 对箭头进行设置
    updateBtnsFlickers();
}

//取走一个B货物
void CenterWidget::takeGoodB()
{
    if(!widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood())
    {
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Ok);
        return ;
    }

    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->setHasGood(false);
    nextTakeColumnB+=1;
    if(nextTakeColumnB >= column - endPoints.at(nextTakeRowB))
    {
        if(endPoints.at(nextTakeRowB) !=0)
        {
            endPoints[nextTakeRowB] = 0;
        }
        nextTakeRowB+=1;
        nextTakeColumnB = 0;
        if(nextTakeRowB>=row){
            nextTakeRowB = rowA;
        }

        if(!widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood())
        {
            //如果首个位置没有货物，判断是否该行其他位置有货，指向该行剩余的
            for(int columnIndex = nextTakeColumnB;columnIndex< column-endPoints.at(nextTakeRowB);++columnIndex)
            {
                if(widgetGoods.at(nextTakeRowB*column+columnIndex)->hasGood()){
                    nextTakeColumnB = columnIndex;
                    break;
                }
            }
        }
    }

    save();
    //对按钮状态进行设置 对箭头进行设置
    updateBtnsFlickers();
}

//更新按钮、闪烁
void CenterWidget::updateBtnsFlickers()
{
    for(int rowindex=0;rowindex<row;++rowindex)
    {
        //查看该行有多少货物
        int goodAmount = 0;
        for(int k=column*rowindex;k<column*(rowindex+1);++k)
        {
            if(widgetGoods.at(k)->hasGood())++goodAmount;
        }

        if(goodAmount == 0){
            fillButtons.at(rowindex)->setEnabled(true);
            fillButtons.at(rowindex)->setVisible(true);
        }else{
            fillButtons.at(rowindex)->setEnabled(false);
            fillButtons.at(rowindex)->setVisible(false);
        }

        if(goodAmount>0 && goodAmount < column && endPoints.at(rowindex)==0){
            translationButtons.at(rowindex)->setEnabled(true);
            translationButtons.at(rowindex)->setVisible(true);
        }else{
            translationButtons.at(rowindex)->setEnabled(false);
            translationButtons.at(rowindex)->setVisible(false);
        }

        for(int columnindex = 0;columnindex<column;++columnindex)
        {
            if((rowindex==nextTakeRowA&&columnindex==nextTakeColumnA) ||(rowindex == nextTakeRowB && columnindex==nextTakeColumnB)){
                widgetGoods.at(rowindex*column+columnindex)->setFlicker(true);
            }else{
                widgetGoods.at(rowindex*column+columnindex)->setFlicker(false);
            }
        }
    }
}

//保存状态到配置文件
void CenterWidget::save()
{
    //需要保存的内容如下：
    //1、row / column
    //    configure.setValue("solution"+solutionPrefix+"/row",row);
    //    configure.setValue("solution"+solutionPrefix+"/column",column);

    //2、endpoint
    for(int i=0;i<row;++i){
        configure.setValue(QString("solution"+solutionPrefix+"/endpoint/%1").arg(i),endPoints.at(i));
    }

    //3、nextRowA/nextColumnA
    configure.setValue("solution"+solutionPrefix+"/nextTakeColumnA",nextTakeColumnA);
    configure.setValue("solution"+solutionPrefix+"/nextTakeRowA",nextTakeRowA);

    //4、nextRowB/nextColumnB
    configure.setValue("solution"+solutionPrefix+"/nextTakeColumnB",nextTakeColumnB);
    configure.setValue("solution"+solutionPrefix+"/nextTakeRowB",nextTakeRowB);

    //5、每个点位是否有货  hasgood
    for(int i=0;i<row;++i){
        for(int j=0;j<column;++j){
            configure.setValue(QString("solution"+solutionPrefix+"/hasGood/%1/%2").arg(i).arg(j),widgetGoods.at(i*column+j)->hasGood());
        }
    }
    //保存到配置文件
    configure.save();
}
