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
    if(!rf.init())
    {
        return false;
    }

    //初始化车辆
    int quantity = configure.getValue("agv/quantity").toInt();
    for(int i=0;i<quantity;++i){

        int id = i+1;
        QString ipKey = QString("agv/agv_ip_%1").arg(id);
        QString portKey = QString("agv/agv_port_%1").arg(id);
        QString ip = configure.getValue(ipKey).toString();
        int port = configure.getValue(portKey).toInt();

        if(ip.length()>0 && port>0){
            AgvConnector *agv = new AgvConnector;
            agv->init(id,ip,port);
            connect(agv,SIGNAL(finish(int)),this,SLOT(onTaskFinish(int)));
            agvs.push_back(agv);
        }else{
            qDebug() << "agv agv_ip_"<<id<<" config error!";
        }
    }

    //任务轮询定时器
    tasktimer.setInterval(1000);
    connect(&tasktimer,SIGNAL(timeout()),this,SLOT(onTaskCheck()));
    tasktimer.start();
    return true;
}

void ControlCenter::onButtn(int address)
{
    static int kk = 0;
    if(address == 0x81 && task81Finish)
    {
        //产生一个A任务
        task81Finish = false;
        Task task;
        task.line = Task::LineA;
        task.task_createTime = QDateTime::currentDateTime();

        //存库以获取ID
        QString insertSql = "INSERT INTO agv_task (task_line,task_createTime) VALUES (?,?);SELECT @@Identity;";
        QStringList params;
        params<<QString("%1").arg(task.line)<<task.task_createTime.toString(DATE_TIME_FORMAT);
        QList<QStringList> insertResult = g_sql->query(insertSql,params);
        if(insertResult.length()>0&&insertResult.at(0).length()>0)
        {
            task.id=(insertResult.at(0).at(0).toInt());
            todoTasks.push_back(task);
        }else{
            //存库失败！
            QMessageBox::critical(NULL,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Ok);
        }
    }else if(address == 0x82 && task82Finish)
    {
        //产生一个B任务
        task82Finish = false;
        Task task;
        task.line = Task::LineB;
        task.task_createTime = QDateTime::currentDateTime();

        //TODO 存库并获取ID
        QString insertSql = "INSERT INTO agv_task (task_line,task_createTime) VALUES (?,?);SELECT @@Identity;";
        QStringList params;
        params<<QString("%1").arg(task.line)<<task.task_createTime.toString(DATE_TIME_FORMAT);
        QList<QStringList> insertResult = g_sql->query(insertSql,params);
        if(insertResult.length()>0&&insertResult.at(0).length()>0)
        {
            task.id=(insertResult.at(0).at(0).toInt());
            todoTasks.push_back(task);
        }else{
            //存库失败！
            QMessageBox::critical(NULL,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Ok);
        }
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
    if(agv->sendTask(todoTasks.at(0).id,todoTasks.at(0).line,station))
    {
        Task t = todoTasks.at(0);
        t.station = station;
        t.excuteAgv = agv->getId();
        t.task_excuteTime = QDateTime::currentDateTime();
        //更新任务执行时间/执行的车辆和站点
        QString updateSql = "update agv_task set agv_station=?,excuteAgv=?,task_excuteTime=?  where id=?";;
        QStringList params;
        params<<QString("%1").arg(t.station)
             <<QString("%1").arg(t.excuteAgv)
            <<t.task_excuteTime.toString(DATE_TIME_FORMAT)
           <<QString("%1").arg(t.id);

        if(!g_sql->exeSql(updateSql,params)){
            //更新数据库失败
            QMessageBox::critical(NULL,QStringLiteral("错误"),QStringLiteral("对任务数据库更新失败"),QMessageBox::Ok);
        }

        doingTasks.append(t);
        todoTasks.removeAt(0);

        //开始执行，便将货物从UI界面中移除。
        if(t.line == Task::LineA)
            centerWidget->takeGoodA();
        else
            centerWidget->takeGoodA();
    }
}

void ControlCenter::onTaskFinish(int taskId)
{
    for(int i=0;i<doingTasks.length();++i)
    {
        Task t = doingTasks.at(i);
        if(t.id == taskId)
        {
            if(t.line == Task::LineA)
            {
                task81Finish = true;
            }else{
                task82Finish = true;
            }
            t.task_finishTime = QDateTime::currentDateTime();

            //更新任务完成时间
            QString updateSql = "update agv_task set task_finishTime=? where id=?";;
            QStringList params;
            params<<t.task_excuteTime.toString(DATE_TIME_FORMAT)
                 <<QString("%1").arg(t.id);

            if(!g_sql->exeSql(updateSql,params))
            {
                //更新数据库失败
                QMessageBox::critical(NULL,QStringLiteral("错误"),QStringLiteral("对任务数据库更新失败"),QMessageBox::Ok);
            }

            doingTasks.removeAt(i);
            break;
        }
    }
}
