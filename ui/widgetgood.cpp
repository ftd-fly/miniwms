#include "widgetgood.h"
#include "global.h"

WidgetGood::WidgetGood(int _code, int _type, int _iHasGood, bool _needRotate, QWidget *parent) : QWidget(parent),
    code(_code),
    iHasGood(_iHasGood),
    needRotate(_needRotate),
    mouseover(false),
    type(_type),
    statusOn(false)
{
    if(!needRotate)
        this->setFixedSize(configure.getValue("ui/good_width").toInt(),configure.getValue("ui/good_height").toInt());
    else
        this->setFixedSize(configure.getValue("ui/good_height").toInt(),configure.getValue("ui/good_width").toInt());

    this->setToolTip(QString(QStringLiteral("堆放点%1_%2_%3")).arg(_type==1?"A":"B").arg(1+((code-1)/column)).arg(1+(code-1)%column));

    if(iHasGood>0){
        status = GOOD_STATUS_YES;
    }else{
        status = GOOD_STATUS_NO;
    }

    statusTimer.setInterval(800);
    connect(&statusTimer,&QTimer::timeout,this,&WidgetGood::onStatusTimer);

    connect(&clicktimer, &QTimer::timeout,this,&WidgetGood::onClicked);
}

void WidgetGood::setHasGood(int _iHasGood){
    iHasGood=_iHasGood;
    if(iHasGood>0){
        status = GOOD_STATUS_YES;
    }else{
        status = GOOD_STATUS_NO;
    }
    update();
}

void WidgetGood::setStatus(int _status)
{
    if(_status>=GOOD_STATUS_NO && _status<=GOOD_STATUS_TOTAKE)
    {
        status = _status;
        //两个状态不需要闪烁
        if(status == GOOD_STATUS_NO || status == GOOD_STATUS_YES)
        {
            statusTimer.stop();
            statusOn = false;
            update();
        }
        //三个状态需要闪烁
        else if(status == GOOD_STATUS_TAKING){
            statusTimer.setInterval(450);
            statusTimer.start();
            update();
        }else if(status == GOOD_STATUS_TOPUT){
            statusTimer.setInterval(800);
            statusTimer.start();
            update();
        }else if(status == GOOD_STATUS_TOTAKE){
            statusTimer.setInterval(800);
            statusTimer.start();
            update();
        }
    }
}


void WidgetGood::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();

    //画轮廓
    QPen pen;
    if(mouseover){
        pen.setWidth(1);
        pen.setColor("yellow");
    }else{
        pen.setWidth(0);
    }

    QColor c("darkgray");
    if(status == GOOD_STATUS_NO)c = QColor("darkgray");
    else if(status == GOOD_STATUS_YES)c = QColor("#0070C0");
    else if(status == GOOD_STATUS_TAKING && statusOn)c = QColor("green");
    else if(status == GOOD_STATUS_TOPUT&& statusOn)c = QColor("gold");
    else if(status == GOOD_STATUS_TOTAKE){
        c = QColor("#0070C0");
        if(statusOn){
            pen.setWidth(8);
            pen.setColor("red");
        }
    }
    painter.setPen(pen);
    painter.setBrush(c);
    QRect rectTemp = this->rect();
    if(!mouseover)
        rectTemp.adjust(-1,-1,1,1);
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
    font.setPointSize(14);
    // 设置字符间距
    font.setLetterSpacing(QFont::AbsoluteSpacing, 5);

    // 使用字体
    painter.setFont(font);
    QString text;
    if(type==1)text= QString("A");
    if(type==2)text= QString("B");
    if(iHasGood<=0){
        text+= QString("%1_%2").arg(1+(code-1)/column).arg(1+(code-1)%column);
        pen.setColor("black");
        painter.setPen(pen);
        painter.drawText(rect(),Qt::AlignCenter, text);
    }else{
        text+= QString("[%1]").arg(iHasGood);
        pen.setColor("red");
        painter.setPen(pen);
        painter.drawText(rect(),Qt::AlignCenter, text);
    }

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

void WidgetGood::onStatusTimer()
{
    statusOn = !statusOn;
    update();
}

void WidgetGood::onClicked()
{
    clicktimer.stop();
    //单击事件
    if(iHasGood<=0 && status == GOOD_STATUS_TOPUT)
    {
        QString text;
        if(type==1)text= QString("A");
        if(type==2)text= QString("B");

        text+= QString("%1_%2").arg(1+(code-1)/column).arg(1+(code-1)%column);
        //左键单机，进行添加
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确定摆放"),QStringLiteral("确认在")+text+QStringLiteral("摆放货物？"),QMessageBox::Yes | QMessageBox::No, this);
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
            if(type==1){
                centerWidget->addGood((code-1)/column,(code-1)%column);
            }else{
                centerWidget->addGood(rowA+(code-1)/column,(code-1)%column);
            }
        }
    }else if(iHasGood > 0 && (status == GOOD_STATUS_YES||status == GOOD_STATUS_TOTAKE)){
        //点击删除
        QString text;
        if(type==1)text= QString("A");
        if(type==2)text= QString("B");

        text+= QString("%1_%2").arg(1+(code-1)/column).arg(1+(code-1)%column);

        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("确定摆放"),QStringLiteral("确认删除")+text+QStringLiteral("的货物？"),QMessageBox::Yes | QMessageBox::No, this);
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
            if(type==1){
                centerWidget->removeGood((code-1)/column,(code-1)%column);
            }else{
                centerWidget->removeGood(rowA+(code-1)/column,(code-1)%column);
            }
        }
    }

}

void WidgetGood::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton){
        //800ms之内没有变成双击事件，就认为是单击事件
        clicktimer.start(800);
    }
}

void WidgetGood::mouseDoubleClickEvent(QMouseEvent * event)
{
    clicktimer.stop();
    //左键双击,直接添加
    if(event->button() == Qt::LeftButton && iHasGood<=0 && status == GOOD_STATUS_TOPUT)
    {

        if(type==1){
            centerWidget->addGood((code-1)/column,(code-1)%column);
        }else{
            centerWidget->addGood(rowA+(code-1)/column,(code-1)%column);
        }
    }
}
