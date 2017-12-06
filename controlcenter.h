#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include <QObject>

class ControlCenter : public QObject
{
    Q_OBJECT
public:
    explicit ControlCenter(QObject *parent = nullptr);

signals:

public slots:
};

#endif // CONTROLCENTER_H