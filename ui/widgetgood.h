#ifndef WIDGETGOOD_H
#define WIDGETGOOD_H

#include <QWidget>
#include <QTimer>
class WidgetGood : public QWidget
{
    Q_OBJECT
public:

    explicit WidgetGood(int _code,int _type,int _iHasGood = 0,bool _needRotate = false,QWidget *parent = nullptr);

    int hasGood(){return iHasGood;}
    void setHasGood(int _iHasGood){iHasGood=_iHasGood;update();}

    void setTakeFlicker(bool f);

    void setPutFlicker(bool f);

signals:

public slots:
    void onTakeFlicker();
    void onPutFlicker();
protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent* e);
private:
    int code;//位置编号
    int iHasGood;//是否有货
    bool needRotate;//是否需要旋转
    bool mouseover;
    int type;
    QTimer takeFlickerTimer;
    bool takeFlickerOn;
    QTimer putFlickerTimer;
    bool putFlickerOn;
};

#endif // WIDGETGOOD_H
