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
    void setHasGood(int _iHasGood);

    void setStatus(int _status);
    int getStatus(){return status;}
signals:

public slots:
    void onStatusTimer();
    void onClicked();
protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent* e);
    void mousePressEvent(QMouseEvent* event);      //单击
    void mouseDoubleClickEvent(QMouseEvent * event);
private:
    int code;//位置编号
    int iHasGood;//是否有货
    bool needRotate;//是否需要旋转
    bool mouseover;
    int type;

    QTimer statusTimer;
    bool statusOn;

    QTimer clicktimer;   //定时器
public:
    enum{
        GOOD_STATUS_NO = 0,//无货状态// 固定灰色
        GOOD_STATUS_YES = 1,//有货状态// 固定蓝色
        GOOD_STATUS_TAKING = 2,//取货中状态//蓝色 灰色交替
        GOOD_STATUS_TOPUT = 3,//可放货状态//黄色 灰色交替
        GOOD_STATUS_TOTAKE = 4,//可取货状态//蓝色 红色边框
    };
private:
    int status;
};

#endif // WIDGETGOOD_H
