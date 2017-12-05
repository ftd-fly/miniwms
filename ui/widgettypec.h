#ifndef WIDGETTYPEC_H
#define WIDGETTYPEC_H

#include <QWidget>
#include "widgetgood.h"
class QPushButton;
class QLabel;

class WidgetTypeC : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetTypeC(QWidget *parent = nullptr);

    //取走一个A货物
    void takeGoodA();

    //取走一个B货物
    void takeGoodB();


signals:

public slots:

private slots:
    //填满一行A货物
    void fillRow();

    //平移一行货物
    void translation();
private:
    void updateBtnsArrows();

    QList<WidgetGood *> widgetGoods;

    //标记下一个取货A的点
    int nextTakeColumnA;
    int nextTakeRowA;

    //标记下一个取货B的点
    int nextTakeRowB;
    int nextTakeColumnB;


    QList<QLabel *> indexLabels;

    QList<QPushButton *> fillButtons;//补满一行货

    QList<QPushButton *> translationButtons;//平移一行货A

    int row;
    int column;

    QList<int> endPoints;//考虑到平移的问题，用于标记该行是否执行到底，
};

#endif // WIDGETTYPEC_H
