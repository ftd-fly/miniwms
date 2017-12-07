#include "controlcenter.h"
#include "global.h"

ControlCenter::ControlCenter(QObject *parent) : QObject(parent),
    task81Finish(true),
    task82Finish(true)
{

}

bool ControlCenter::init()
{
    //初始化串口
    connect(&rf,SIGNAL(buttonClick(int)),this,SLOT(onButtn(int)));
    if(!rf.init()){
        return false;
    }
    //初始化车辆
    int quantity = configure.getValue("agv/quantity").toInt();
    for(int i=0;i<quantity;++i){

        int id = i+1;
        QString ip = configure.getValue(QString("agv/agv_ip_").arg(id)).toString();
        int port = configure.getValue(QString("agv/agv_port_").arg(id)).toInt();

        if(ip.length()>0 && port>0){
            AgvConnector *agv = new AgvConnector;
            agv->init(id,ip,port);
            agvs.push_back(agv);
        }else{
            qDebug() << "agv agv_ip_"<<id<<" config error!";
        }
    }

    //任务轮询定时器
    tasktimer.setInterval(1000);
    connect(&tasktimer,SIGNAL(timeout()),this,SLOT(onTaskCheck()));

    return true;
}

void ControlCenter::onButtn(int address)
{
    if(address == 0x81 && task81Finish)
    {
        //产生一个A任务
        task81Finish = false;
        Task task;
        task.line = Task::LineA;
        task.task_createTime = QDateTime::currentDateTime();

        //TODO 存库以获取ID

        todoTasks.push_back(task);
    }else if(address == 0x82 && task82Finish)
    {
        //产生一个B任务
        task82Finish = false;
        Task task;
        task.line = Task::LineB;
        task.task_createTime = QDateTime::currentDateTime();

        //TODO 存库以获取ID
        todoTasks.push_back(task);

    }
}

void ControlCenter::onTaskCheck()
{
    //查询当前待做任务的队列
    if(todoTasks.length()<=0)return;

    //查询车辆执行的任务的状态，是否有空闲
    AgvConnector *agv = NULL;
    for(QList<AgvConnector *>::iterator itr = agvs.begin();itr!=agvs.end();++itr){
        if((*itr)->isIdle()){
            agv = *itr;
            break;
        }
    }

    //没有空闲车辆
    if(agv==NULL)return ;


    //获取下一个取货点
    int station;
    if(todoTasks.at(0).line == Task::LineA){
        station = getNextAStation();
    }else{
        station = getNextBStation();
    }
    if(station == -1)return ;

    //执行
    if(agv->sendTask(todoTasks.at(0).line,station))
    {
        Task t = todoTasks.at(0);
        t.station = station;
        t.excuteAgv = agv->getId();
        t.task_excuteTime = QDateTime::currentDateTime();
        doingTasks.append(t);
        todoTasks.removeAt(0);
    }
}

void ControlCenter::onTaskFinish(int type)
{

}
