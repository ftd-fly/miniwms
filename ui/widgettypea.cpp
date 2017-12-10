#include <assert.h>
#include "widgettypea.h"
#include "global.h"

WidgetTypeA::WidgetTypeA(QWidget *parent) : CenterWidget(parent)
{
    init();
}

//将按钮、货物，摆放在合适的位置
void WidgetTypeA::initGoodPosition()
{
    //方案A。
    //左边两个按钮。货物从右向左，从上往下排列

    assert(row == 4 && rowA==2);
    QGridLayout *gridlayout = new QGridLayout;

    //第一行A
    gridlayout->addWidget(translationButtons.at(0),0,0,1,1,Qt::AlignCenter);
    gridlayout->addWidget(fillButtons.at(0),0,1,1,1,Qt::AlignCenter);
    for(int i=0;i<column;++i){
        gridlayout->addWidget(widgetGoods.at(column-i-1),0,2+i,1,1,Qt::AlignCenter);
    }

    //两行A中间的空白区域
    gridlayout->addItem(new QSpacerItem(configure.getValue("ui/good_width").toInt(),configure.getValue("ui/good_height").toInt()),1,0,2,column,Qt::AlignCenter);


    //第二行A
    gridlayout->addWidget(translationButtons.at(1),3,0,1,1,Qt::AlignCenter);
    gridlayout->addWidget(fillButtons.at(1),3,1,1,1,Qt::AlignCenter);
    for(int i=0;i<column;++i){
        gridlayout->addWidget(widgetGoods.at(1*column+column-i-1),3,2+i,1,1,Qt::AlignCenter);
    }

    //第一行B
    gridlayout->addWidget(translationButtons.at(2),4,0,1,1,Qt::AlignCenter);
    gridlayout->addWidget(fillButtons.at(2),4,1,1,1,Qt::AlignCenter);
    for(int i=0;i<column;++i){
        gridlayout->addWidget(widgetGoods.at(2*column+column-i-1),4,2+i,1,1,Qt::AlignCenter);
    }

    //两行B中间的空白区域
    gridlayout->addItem(new QSpacerItem(configure.getValue("ui/good_width").toInt(),configure.getValue("ui/good_height").toInt()),5,0,2,column,Qt::AlignCenter);


    //第二行B
    gridlayout->addWidget(translationButtons.at(3),7,0,1,1,Qt::AlignCenter);
    gridlayout->addWidget(fillButtons.at(3),7,1,1,1,Qt::AlignCenter);
    for(int i=0;i<column;++i){
        gridlayout->addWidget(widgetGoods.at(3*column+column-i-1),7,2+i,1,1,Qt::AlignCenter);
    }

    gridlayout->setSpacing(configure.getValue("ui/good_spacing").toInt());
    int margin = configure.getValue("ui/good_margin").toInt();
    gridlayout->setContentsMargins(margin,margin,margin,margin);
    centergroup->setLayout(gridlayout);

}
