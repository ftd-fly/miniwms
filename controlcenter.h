#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include <QObject>
#include "radiofrequency.h"
#include "agvconnector.h"
#include "task.h"

class ControlCenter : public QObject
{
    Q_OBJECT
public:
    explicit ControlCenter(QObject *parent = nullptr);
    bool init();
signals:

public slots:
    void onButtn(int address);
    void onTaskCheck();
    void onTaskFinish(int taskId);
    void onError();
private:
    RadioFrequency rf;
    bool task81Finish;
    bool task82Finish;
    QList<Task> todoTasks;
    QList<Task> doingTasks;
    QTimer tasktimer;
    QList<AgvConnector *> agvs;
};

#endif // CONTROLCENTER_H
