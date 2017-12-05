#ifndef WIDGETGOOD_H
#define WIDGETGOOD_H

#include <QWidget>

class WidgetGood : public QWidget
{
    Q_OBJECT
public:

    explicit WidgetGood(int _code,int _type,bool _bHasGood = true,bool _needRotate = false,QWidget *parent = nullptr);

    bool hasGood(){return bHasGood;}
    void setHasGood(bool _bHasGood){bHasGood=_bHasGood;update();}
signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent* e);
private:
    int code;//位置编号
    bool bHasGood;//是否有货
    bool needRotate;//是否需要旋转
    bool mouseover;
    int type;
};

#endif // WIDGETGOOD_H
