#ifndef QYHCLICKLABEL_H
#define QYHCLICKLABEL_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class QyhClickLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QyhClickLabel(QString txt,QWidget *parent = nullptr);
    void setFlicker(bool f);
signals:

private slots:
    void onFlickTimer();
private:
    void upadteAppearance();
    QTimer flickTimer;
    bool isOn = false;
};

#endif // QYHCLICKLABEL_H
