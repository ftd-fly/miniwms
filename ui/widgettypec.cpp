#include "widgettypec.h"
#include "global.h"

WidgetTypeC::WidgetTypeC(QWidget *parent) : CenterWidget(parent)
{
    init();
}

//将按钮、货物，摆放在合适的位置
void WidgetTypeC::initGoodPosition()
{
    //方案C。
    //左边两个按钮。货物从右向左，从上往下排列
    QGridLayout *gridlayout = new QGridLayout;

    for(int j=0;j<row;++j)
    {       
        for(int i=column;i>0;--i)
        {
            gridlayout->addWidget(widgetGoods.at(j*column+column-i),j,i,1,1,Qt::AlignCenter);
        }
    }

    gridlayout->setSpacing(configure.getValue("ui/good_spacing").toInt());
    int margin = configure.getValue("ui/good_margin").toInt();
    gridlayout->setContentsMargins(margin,margin,margin,margin);
    centergroup->setLayout(gridlayout);

}


