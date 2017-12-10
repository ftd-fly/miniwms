#ifndef WIDGETTYPEB_H
#define WIDGETTYPEB_H

#include <QWidget>
#include "centerwidget.h"

class WidgetTypeB : public CenterWidget
{
    Q_OBJECT
public:
    explicit WidgetTypeB(QWidget *parent = nullptr);

signals:

public slots:

protected:
    //将按钮、货物，摆放在合适的位置
    void initGoodPosition();
};

#endif // WIDGETTYPEB_H
