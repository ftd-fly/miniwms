#ifndef TASK_H
#define TASK_H

#include <QDateTime>

class Task
{
public:
    enum{
        LineA = 0,//代表了产线1
        LineB = 1,//代表了产线2
    };
    Task():id(-1) {

    }
    Task(const Task& b){
        id = b.id;
        line = b.line;
        excuteAgv = b.excuteAgv;
        station = b.station;
        if(b.task_createTime.isValid()){
            task_createTime = b.task_createTime;
        }
        if(b.task_excuteTime.isValid()){
            task_excuteTime = b.task_excuteTime;
        }
        if(b.task_finishTime.isValid()){
            task_finishTime = b.task_finishTime;
        }
    }
    bool operator <(const Task &b){
        return id<b.id;
    }

    int id;
    int line;
    int excuteAgv;
    int station;
    QDateTime task_createTime;
    QDateTime task_excuteTime;
    QDateTime task_finishTime;
};


#endif // TASK_H
