#include "widgettypec.h"
#include "global.h"

#include "widgetgood.h"

WidgetTypeC::WidgetTypeC(QWidget *parent) : QWidget(parent)
{
    row = configure.getValue("solution/c_row").toInt();
    column = configure.getValue("solution/c_column").toInt();

    //这个理论上是从配置文件读取的//这里先给赋值
    nextTakeColumnA = column-1;//下一个货A的点的列
    nextTakeRowA = 0;
    nextTakeColumnB = column-1;
    nextTakeRowB = 3;

    //endpoint也应该从配置文件读取
    endPoints.clear();
    for(int i=0;i<row;++i){
        endPoints.append(0);
    }

    //是否有货也应该从配置中读取...

    //1.从右向左添加
    //2.A类3行，剩下的是B类的
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addStretch(1);

    for(int j=0;j<row;++j){
        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addStretch(1);
        //添加两个按钮
        QPushButton *traBtn = new QPushButton(QStringLiteral("该行货物平移→"));
        translationButtons.append(traBtn);
        connect(traBtn,SIGNAL(clicked(bool)),this,SLOT(translation()));
        hlayout->addWidget(traBtn);

        QPushButton *fillBtn = new QPushButton(QStringLiteral("该行货物已补满"));
        fillButtons.append(fillBtn);
        connect(fillBtn,SIGNAL(clicked(bool)),this,SLOT(fillRow()));

        hlayout->addWidget(fillBtn);

        for(int i=column;i>0;--i)
        {
            int type = 1;
            if(j>=3)type=2;
            int id = i+j*column;
            if(type==2)id=i+(j-3)*column;
            WidgetGood *good = new WidgetGood(id,type,false);
            hlayout->addWidget(good);
            widgetGoods.append(good);
        }

        //添加一个label
        QLabel *indexLabel = new QLabel(QStringLiteral("←"));
        QFont ft;
        ft.setPointSize(32);
        indexLabel->setFont(ft);
        indexLabels.append(indexLabel);
        hlayout->addWidget(indexLabel);


        hlayout->addStretch(1);
        hlayout->setMargin(10);
        hlayout->setSpacing(5);
        vlayout->addItem(hlayout);
    }
    vlayout->addStretch(2);
    vlayout->setMargin(10);
    vlayout->setSpacing(5);

    setLayout(vlayout);

    updateBtnsArrows();

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
        updateBtnsArrows();
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
        updateBtnsArrows();
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
        nextTakeRowA+=1;
        if(nextTakeRowA>=3){
            nextTakeRowA = 0;
        }
    }
    //对按钮状态进行设置 对箭头进行设置
    updateBtnsArrows();
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
        nextTakeRowB+=1;
        if(nextTakeRowB>=3){
            nextTakeRowB = 0;
        }
    }

    //对按钮状态进行设置 对箭头进行设置
    updateBtnsArrows();
}

void WidgetTypeC::updateBtnsArrows()
{
    for(int rowindex=0;rowindex<row;++rowindex){
        if(rowindex == nextTakeRowA || rowindex==nextTakeRowB){
            indexLabels.at(rowindex)->setDisabled(false);
            indexLabels.at(rowindex)->setStyleSheet("color:red");//文本颜色
        }else{
            indexLabels.at(rowindex)->setDisabled(true);
            indexLabels.at(rowindex)->setStyleSheet("color:grey");//文本颜色
        }
        //查看该行有多少货物
        int goodAmount = 0;
        for(int k=column*rowindex;k<column*(rowindex+1);++k)
        {
            if(widgetGoods.at(k)->hasGood())++goodAmount;
        }

        if(goodAmount == 0){
            fillButtons.at(rowindex)->setEnabled(true);
        }else{
            fillButtons.at(rowindex)->setEnabled(false);
        }

        if(goodAmount>0 && goodAmount < column && endPoints.at(rowindex)==0){
            translationButtons.at(rowindex)->setEnabled(true);
        }else{
            translationButtons.at(rowindex)->setEnabled(false);
        }
    }

}
