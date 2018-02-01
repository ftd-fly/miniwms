#include <assert.h>
#include "widgettypeb.h"
#include "global.h"

WidgetTypeB::WidgetTypeB(QWidget *parent) : CenterWidget(parent)
{
    init();
}
//将按钮、货物，摆放在合适的位置
void WidgetTypeB::initGoodPosition()
{
//    //方案B。
//    //左边两个按钮。货物从右向左，从上往下排列
//    QGridLayout *gridlayout = new QGridLayout;

//    assert(rowA==row-rowA);
//    //A货物
//    for(int i=0;i<rowA;++i)
//    {
//        for(int j=0;j<column;++j)
//        {
//            gridlayout->addWidget(widgetGoods.at(i*column+column-j-1),j,rowA-i-1,1,1,Qt::AlignCenter);
//        }
//        gridlayout->addWidget(fillButtons.at(i),3,rowA-i-1,1,1,Qt::AlignCenter);
//        gridlayout->addWidget(translationButtons.at(i),4,rowA-i-1,1,1,Qt::AlignCenter);
//    }
//    //中间分割线
//    gridlayout->addItem(new QSpacerItem(configure.getValue("ui/good_width").toInt(),configure.getValue("ui/good_height").toInt()),5,0,1,rowA,Qt::AlignCenter);

//    //B货物
//    for(int i=0;i<rowA;++i)
//    {
//        gridlayout->addWidget(translationButtons.at(i+rowA),6,rowA-i-1,1,1,Qt::AlignCenter);
//        gridlayout->addWidget(fillButtons.at(i+rowA),7,rowA-i-1,1,1,Qt::AlignCenter);
//        for(int j=0;j<column;++j)
//        {
//            gridlayout->addWidget(widgetGoods.at(rowA*column+i*column+j),8+j,rowA-i-1,1,1,Qt::AlignCenter);
//        }
//    }

//    gridlayout->setSpacing(configure.getValue("ui/good_spacing").toInt());
//    int margin = configure.getValue("ui/good_margin").toInt();
//    gridlayout->setContentsMargins(margin,margin,margin,margin);
//    centergroup->setLayout(gridlayout);

}
