#ifndef WIDGETTYPEA_H
#define WIDGETTYPEA_H

#include <QWidget>
#include "centerwidget.h"

class WidgetTypeA : public CenterWidget
{
    Q_OBJECT
public:
    explicit WidgetTypeA(QWidget *parent = nullptr);

signals:

public slots:

protected:
    //将按钮、货物，摆放在合适的位置
    void initGoodPosition();
};

#endif // WIDGETTYPEA_H
