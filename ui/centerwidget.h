#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QWidget>
#include <QTimer>
#include "qyhclicklabel.h"
class QPushButton;
class QLabel;
class QGroupBox;
class QLCDNumber;

class CenterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CenterWidget(QWidget *parent = nullptr);

    int getNextAStation();

    int getNextBStation();

    void addGood(int add_row,int add_column);

    void removeGood(int remove_row,int remove_column);

signals:

public slots:
    //取走一个A货物
    void takeGoodA();

    //取走一个B货物
    void takeGoodB();

    void onBtnA();

    void onBtnB();

protected:
    //将按钮、货物，摆放在合适的位置
    virtual void initGoodPosition() = 0;

    //初始化
    void init();
private slots:
    //查询完成数量
    void queryNumber();

    void onStartTakeA();

    void onStartTakeB();

    void onFinishTakeA();

    void onFinishTakeB();
protected:

    QGroupBox * centergroup;

private:
    int nextTakeRowA = -1;
    int nextTakeColumnA = -1;
    int nextTakeRowB = -1;
    int nextTakeColumnB = -1;

    int nextPutRowA = -1;
    int nextPutColumnA = -1;
    int nextPutRowB = -1;
    int nextPutColumnB = -1;

    int minA = 0;//下一个要取A货物的ID 用于计算下一个要取A的位置
    int minB = 0;//下一个要取B货物的ID 用于计算下一个要取B的位置
    int maxA = 0;//当前A的最大ID值，用于计算下一个添加的A的ID（=maxA+1）
    int maxB = 0;//当前B的最大ID值，用于计算下一个添加的A的ID（=max+1）

    //保存到配置文件
    void save();

    //获取下一个要去的A和B的坐标
    void updateNext();

    QLCDNumber *tAll;
    QLCDNumber *tA;
    QLCDNumber *tB;

    QyhClickLabel *takeABtn;
    QyhClickLabel *takeBBtn;

    QTimer updateNumberTimer;
};

#endif // CENTERWIDGET_H
