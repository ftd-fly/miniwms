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

    int getNextAStation();
    int getNextBStation();
signals:

public slots:

private slots:
    //填满一行A货物
    void fillRow();

    //平移一行货物
    void translation();
private:
    void save();

    void updateBtnsArrowsFlickers();

    QList<QLabel *> indexLabels;

    QList<QPushButton *> fillButtons;//补满一行货

    QList<QPushButton *> translationButtons;//平移一行货A
};

#endif // WIDGETTYPEC_H
