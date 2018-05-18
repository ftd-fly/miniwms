#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include <QObject>
#include "serialthread.h"
#include "agvconnector.h"
#include "task.h"

class ControlCenter : public QObject
{
    Q_OBJECT
public:
    explicit ControlCenter(QObject *parent = nullptr);
    bool init();
    //取消任务 取消任务会暂停未进行的任务的执行，直到调用恢复
    void cancelTask(int aOrB);
    void recover();
//    void testContinue();
signals:
    void sig_light_on(int address);
    void sig_light_off(int address);
public slots:
    void onButtn(int address);
    void onTaskCheck();
    void onTaskFinish(int taskId);
    void onError();
private:
    SerialThread *rf;
    bool taskAFinish;
    bool taskBFinish;
    QList<Task> todoTasks;
    QList<Task> doingTasks;
    QTimer tasktimer;
    QList<AgvConnector *> agvs;
};

#endif // CONTROLCENTER_H
