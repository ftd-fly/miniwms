#include "qyhclicklabel.h"
#include <QMouseEvent>

QyhClickLabel::QyhClickLabel(QString txt, QWidget *parent) : QLabel(txt,parent)
{
    flickTimer.setInterval(800);
    connect(&flickTimer,&QTimer::timeout,this,&QyhClickLabel::onFlickTimer);
    upadteAppearance();
}

void QyhClickLabel::setFlicker(bool f)
{
    if(!f){
        flickTimer.stop();
        isOn = false;
        upadteAppearance();
    }else{
        flickTimer.start();
    }
}

void QyhClickLabel::onFlickTimer()
{
    isOn = !isOn;
    upadteAppearance();
}

void QyhClickLabel::upadteAppearance()
{
    QString defaultStyle ="border:1px solid ADADAD;"
                          "padding-left:30px;"
                          "padding-right:30px;"
                          "padding-top:10px;"
                          "padding-bottom:10px;"
                          "font:20px;";

    if(!isOn){
        defaultStyle += "background-color:green;";
    }else{
        defaultStyle += "background-color:yellow;";
    }

    setStyleSheet(defaultStyle);
}
