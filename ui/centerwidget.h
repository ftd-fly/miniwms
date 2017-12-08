#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QWidget>
#include <QTimer>
class QPushButton;
class QLabel;
class QGroupBox;
class QLCDNumber;

class CenterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CenterWidget(QWidget *parent = nullptr);

signals:

public slots:
    //取走一个A货物
    void takeGoodA();

    //取走一个B货物
    void takeGoodB();

protected:
    //将按钮、货物，摆放在合适的位置
    virtual void initGoodPosition() = 0;

    //初始化
    void init();
private slots:
    //填满一行货物
    void fillRow();

    //平移一行货物
    void translation();

    //查询完成数量
    void queryNumber();
protected:

    QList<QPushButton *> fillButtons;//补满一行货

    QList<QPushButton *> translationButtons;//平移一行货A

    QGroupBox * centergroup;

private:



    //更新按钮、闪烁
    void updateBtnsFlickers();

    //保存到配置文件
    void save();

    QLCDNumber *tAll;
    QLCDNumber *tA;
    QLCDNumber *tB;

    QTimer updateNumberTimer;
};

#endif // CENTERWIDGET_H
