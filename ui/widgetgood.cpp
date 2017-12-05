#include "widgetgood.h"
#include "global.h"

WidgetGood::WidgetGood(int _code, int _type, bool _bHasGood, bool _needRotate, QWidget *parent) : QWidget(parent),
    code(_code),
    bHasGood(_bHasGood),
    needRotate(_needRotate),
    mouseover(false),
    type(_type)
{
    if(!needRotate)
        this->setFixedSize(configure.getValue("ui/good_width").toInt(),configure.getValue("ui/good_height").toInt());
    else
        this->setFixedSize(configure.getValue("ui/good_height").toInt(),configure.getValue("ui/good_width").toInt());

    this->setToolTip(QString(QStringLiteral("堆放点%1%2")).arg(_type==1?"A":"B").arg(code));
}

void WidgetGood::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();

    //画轮廓

    QPen pen;
    if(mouseover){
        pen.setWidth(5);
        pen.setColor("red");
    }else{
        pen.setWidth(0);
    }
    QColor c("#0070C0");
    if(!bHasGood)c = QColor("grey");
    painter.setPen(pen);
    painter.setBrush(c);
    QRect rectTemp = this->rect();
    if(!mouseover)
        rectTemp.adjust(-2,-2,2,2);
    painter.drawRect(rectTemp);

    //画中间两个
    if(!needRotate){
        painter.fillRect(QRect(4,16,80,26),QColor("#CCCCCC"));
        painter.fillRect(QRect(4,66,80,26),QColor("#CCCCCC"));
    }else{
        painter.fillRect(QRect(16,4,26,80),QColor("#CCCCCC"));
        painter.fillRect(QRect(66,4,26,80),QColor("#CCCCCC"));
    }

    QFont font;
    font.setFamily("黑体");
    // 大小
    font.setPointSize(25);
    // 设置字符间距
    font.setLetterSpacing(QFont::AbsoluteSpacing, 5);

    // 使用字体
    painter.setFont(font);
    QString text;
    if(type==1)text= QString("A");
    if(type==2)text= QString("B");
    text+= QString("%1").arg(code);
    pen.setColor("black");
    painter.setPen(pen);
    painter.drawText(rect(),Qt::AlignCenter, text);

    painter.restore();
    QWidget::paintEvent(event);
}

bool WidgetGood::event(QEvent* event)
{
    if (event->type() == QEvent::Enter){
        mouseover = true;
        update();
    }else if (event->type()==QEvent::Leave){
        mouseover = false;
        update();
    }
    return QWidget::event(event);
}
