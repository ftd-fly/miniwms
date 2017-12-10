#ifndef WIDGETTYPEC_H
#define WIDGETTYPEC_H

#include <QWidget>
#include "centerwidget.h"

class WidgetTypeC : public CenterWidget
{
    Q_OBJECT
public:
    explicit WidgetTypeC(QWidget *parent = nullptr);

signals:

public slots:

protected:
    //将按钮、货物，摆放在合适的位置
    void initGoodPosition();
};

#endif // WIDGETTYPEC_H
