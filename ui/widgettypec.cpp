#include <assert.h>
#include "widgettypec.h"
#include "global.h"
#include "widgetgood.h"

WidgetTypeC::WidgetTypeC(QWidget *parent) : QWidget(parent)
{
    //载入配置：
    //载入行数列数
    row = configure.getValue("solutionc/row").toInt();
    column = configure.getValue("solutionc/column").toInt();

    //载入每行的endpoint
    endPoints.clear();
    for(int i=0;i<row;++i){
        endPoints.push_back(configure.getValue(QString("solutionc/endpoint/%1").arg(i)).toInt());
    }

    //载入当前取货行和列
    nextTakeColumnA = configure.getValue("solutionc/nextTakeColumnA").toInt();
    nextTakeRowA = configure.getValue("solutionc/nextTakeRowA").toInt();
    nextTakeColumnB = configure.getValue("solutionc/nextTakeColumnB").toInt();
    nextTakeRowB = configure.getValue("solutionc/nextTakeRowB").toInt();

    //中间个格子
    QGroupBox * centergroup = new QGroupBox(QStringLiteral("存放区"));
    QGridLayout *gridlayout = new QGridLayout;
    gridlayout->setSpacing(10);
    for(int j=0;j<row;++j)
    {
        //添加两个按钮
        QPushButton *traBtn = new QPushButton(QStringLiteral("该行货物平移→"),this);
        translationButtons.append(traBtn);
        connect(traBtn,SIGNAL(clicked(bool)),this,SLOT(translation()));
        gridlayout->addWidget(traBtn,j,0,1,1,Qt::AlignCenter);

        QPushButton *fillBtn = new QPushButton(QStringLiteral("该行货物已补满"),this);
        fillButtons.append(fillBtn);
        connect(fillBtn,SIGNAL(clicked(bool)),this,SLOT(fillRow()));
        gridlayout->addWidget(fillBtn,j,1,1,1,Qt::AlignCenter);

        for(int i=column;i>0;--i)
        {
            int type = 1;
            if(j>=3)type=2;
            int id = i+j*column;
            if(type==2)id=i+(j-3)*column;
            bool hasGood = configure.getValue(QString("solutionc/hasGood/%1/%2").arg(j).arg(column-i)).toBool();
            WidgetGood *good = new WidgetGood(id,type,hasGood);            
            widgetGoods.append(good);
            gridlayout->addWidget(good,j,2+column - i,1,1,Qt::AlignCenter);
        }
        QLabel *indexLabel = new QLabel(QStringLiteral("←"),this);
        QFont ft;
        ft.setPointSize(32);
        indexLabel->setFont(ft);
        indexLabels.append(indexLabel);
        gridlayout->addWidget(indexLabel,j,2+column,1,1,Qt::AlignCenter);
    }
    gridlayout->setSpacing(configure.getValue("ui/good_spacing").toInt());
    int margin = configure.getValue("ui/good_margin").toInt();
    gridlayout->setContentsMargins(margin,margin,margin,margin);
    centergroup->setLayout(gridlayout);
    //水平居中
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addWidget(centergroup);
    hlayout->addStretch(1);

    //竖直居中
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addStretch(1);
    vlayout->addItem(hlayout);
    vlayout->addStretch(1);

    setLayout(vlayout);

    updateBtnsArrowsFlickers();
}

//填满一行A货物
void WidgetTypeC::fillRow()
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
        updateBtnsArrowsFlickers();
    }
}

//平移一行货物
void WidgetTypeC::translation()
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
        updateBtnsArrowsFlickers();
    }
}

//取走一个A货物
void WidgetTypeC::takeGoodA()
{
    //判断当前位置是否有货
    if(!widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood())
    {
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Ok);
        return ;
    }
    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->setHasGood(false);
    nextTakeColumnA-=1;

    if(nextTakeColumnA < endPoints.at(nextTakeRowA)){
        if(endPoints.at(nextTakeRowA) !=0){
            endPoints[nextTakeRowA] = 0;
        }
        nextTakeRowA+=1;
        if(nextTakeRowA>=3){
            nextTakeRowA = 0;
        }
    }
    save();
    //对按钮状态进行设置 对箭头进行设置
    updateBtnsArrowsFlickers();
}

//取走一个B货物
void WidgetTypeC::takeGoodB()
{
    if(!widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood())
    {
        QMessageBox::critical(this,QStringLiteral("错误"),QStringLiteral("当前位置无货物"),QMessageBox::Ok);
        return ;
    }

    //取走货物，对index进行下移
    widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->setHasGood(false);
    nextTakeColumnB-=1;
    if(nextTakeColumnB < endPoints.at(nextTakeRowB)){
        if(endPoints.at(nextTakeRowB) !=0){
            endPoints[nextTakeRowB] = 0;
        }
        nextTakeRowB+=1;
        if(nextTakeRowB>=row){
            nextTakeRowB = 3;
        }
    }

    save();
    //对按钮状态进行设置 对箭头进行设置
    updateBtnsArrowsFlickers();
}

void WidgetTypeC::updateBtnsArrowsFlickers()
{
    for(int rowindex=0;rowindex<row;++rowindex){
        if(rowindex == nextTakeRowA || rowindex==nextTakeRowB){
            indexLabels.at(rowindex)->setDisabled(false);
            indexLabels.at(rowindex)->setStyleSheet("color:red");//文本颜色
            indexLabels.at(rowindex)->setVisible(true);
        }else{
            indexLabels.at(rowindex)->setDisabled(true);
            indexLabels.at(rowindex)->setStyleSheet("color:grey");//文本颜色
            indexLabels.at(rowindex)->setVisible(false);
        }
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

        for(int columnindex = 0;columnindex<column;++columnindex){
            if((rowindex==nextTakeRowA&&columnindex==nextTakeColumnA) ||(rowindex == nextTakeRowB && columnindex==nextTakeColumnB)){
                widgetGoods.at(rowindex*column+columnindex)->setFlicker(true);
            }else{
                widgetGoods.at(rowindex*column+columnindex)->setFlicker(false);
            }
        }
    }
}

void WidgetTypeC::save()
{
    //需要保存的内容如下：
    //1、row / column
    configure.setValue("solutionc/row",row);
    configure.setValue("solutionc/column",column);
    //2、endpoint
    for(int i=0;i<row;++i){
        configure.setValue(QString("solutionc/endpoint/%1").arg(i),endPoints.at(i));
    }
    //3、nextRowA/nextColumnA
    configure.setValue("solutionc/nextTakeColumnA",nextTakeColumnA);
    configure.setValue("solutionc/nextTakeRowA",nextTakeRowA);
    //4、nextRowB/nextColumnB
    configure.setValue("solutionc/nextTakeColumnB",nextTakeColumnB);
    configure.setValue("solutionc/nextTakeRowB",nextTakeRowB);
    //5、每个点位是否有货  hasgood
    for(int i=0;i<row;++i){
        for(int j=0;j<column;++j){
            configure.setValue(QString("solutionc/hasGood/%1/%2").arg(i).arg(j),widgetGoods.at(i*column+j)->hasGood());
        }
    }

    configure.save();
}

int WidgetTypeC::getNextAStation()
{
    if(widgetGoods.at(nextTakeRowA*column+nextTakeColumnA)->hasGood())
    {
        return  nextTakeRowA*column+nextTakeColumnA;
    }
    return -1;
}

int WidgetTypeC::getNextBStation()
{
    if(widgetGoods.at(nextTakeRowB*column+nextTakeColumnB)->hasGood())
    {
        return  nextTakeRowB*column+nextTakeColumnB;
    }
    return -1;
}
