#include "controlcenter.h"
#include "global.h"

ControlCenter::ControlCenter(QObject *parent) : QObject(parent),
    taskAFinish(true),
    taskBFinish(true)
{

}

bool ControlCenter::init()
{
    //初始化串口
    connect(&rf,SIGNAL(buttonClick(int)),this,SLOT(onButtn(int)));
    if(!rf.init())
        return false;

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
            connect(agv,SIGNAL(error()),this,SLOT(onError()));
            connect(agv,SIGNAL(cancel(int)),this,SLOT(onTaskFinish()));
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

void ControlCenter::cancelTask(int aOrB)
{
    tasktimer.stop();
    //cancel task a
    if(aOrB == RADOI_FREQUENCY_ADDRESS_A)
    {
        for(int i=0;i<doingTasks.length();++i)
        {
            if(doingTasks.at(i).line == Task::LineA){
                Task t = doingTasks.at(i);
                rf.lightOff(RADOI_FREQUENCY_ADDRESS_A);
                foreach (auto a, agvs) {
                    if(a->getId() == t.excuteAgv){
                        a->cancelTask(t.line,t.station);
                    }
                }
            }
            break;
        }
    }
    //cancel task B
    else if(aOrB == RADOI_FREQUENCY_ADDRESS_B){
        for(int i=0;i<doingTasks.length();++i)
        {
            if(doingTasks.at(i).line == Task::LineB)
            {
                Task t = doingTasks.at(i);
                rf.lightOff(RADOI_FREQUENCY_ADDRESS_B);
                foreach (auto a, agvs) {
                    if(a->getId() == t.excuteAgv){
                        a->cancelTask(t.line,t.station);
                    }
                }
                break;
            }
        }
    }
}

//void ControlCenter::testContinue()
//{
//    static int randId = 100000;
//    int tempRowB = 0;
//    int tempColumnB = 1;
//    if(agvs.length()>0)
//        agvs[0]->sendTask(++randId,Task::LineB,(tempRowB+rowA)*column+tempColumnB,true);
//}


void ControlCenter::recover()
{
    tasktimer.start();
}

void ControlCenter::onButtn(int address)
{
    if(address == RADOI_FREQUENCY_ADDRESS_A && taskAFinish)
    {
        //产生一个A任务
        taskAFinish = false;
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
            QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Yes);
            mbox.setStyleSheet(
                        "QPushButton {"
                        "font:30px;"
                        "padding-left:100px;"
                        "padding-right:100px;"
                        "padding-top:40px;"
                        "padding-bottom:40px;"
                        "}"
                        "QLabel { font:30px;}"
                        );
            mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
            mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
            mbox.exec();

            taskAFinish = true;
            rf.lightOff(RADOI_FREQUENCY_ADDRESS_A);
        }
    }else if(address == RADOI_FREQUENCY_ADDRESS_B && taskBFinish)
    {
        //产生一个B任务
        taskBFinish = false;
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
            QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Yes);
            mbox.setStyleSheet(
                        "QPushButton {"
                        "font:30px;"
                        "padding-left:100px;"
                        "padding-right:100px;"
                        "padding-top:40px;"
                        "padding-bottom:40px;"
                        "}"
                        "QLabel { font:30px;}"
                        );
            mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
            mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
            mbox.exec();
            taskBFinish = false;
            rf.lightOff(RADOI_FREQUENCY_ADDRESS_B);
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
    for(int i=0;i<todoTasks.length();++i)
    {
        auto t = todoTasks.at(i);

        int station;
        if(t.line == Task::LineA){
            station = centerWidget->getNextAStation();
        }else{
            station = centerWidget->getNextBStation();
        }
        if(station == -1)continue ;

        if(agv->sendTask(t.id,t.line,station))
        {
            t.station = station;
            t.excuteAgv = agv->getId();
            t.task_excuteTime = QDateTime::currentDateTime();
            //更新任务执行时间/执行的车辆和站点
            QString updateSql = "update agv_task set station=?,excuteAgv=?,task_excuteTime=?  where id=?";;
            QStringList params;
            params<<QString("%1").arg(t.station)
                 <<QString("%1").arg(t.excuteAgv)
                <<t.task_excuteTime.toString(DATE_TIME_FORMAT)
               <<QString("%1").arg(t.id);

            if(!g_sql->exeSql(updateSql,params)){
                //更新数据库失败
                QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Yes);
                mbox.setStyleSheet(
                            "QPushButton {"
                            "font:30px;"
                            "padding-left:100px;"
                            "padding-right:100px;"
                            "padding-top:40px;"
                            "padding-bottom:40px;"
                            "}"
                            "QLabel { font:30px;}"
                            );
                mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
                mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
                mbox.exec();
            }

            doingTasks.append(t);
            todoTasks.removeAt(i);

            //开始执行，便将货物从UI界面中移除。
            if(t.line == Task::LineA){
                centerWidget->takeGoodA();
            }
            else{
                centerWidget->takeGoodB();
            }
            break;
        }
    }
}

void ControlCenter::onError()
{
    QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("AGV发生错误，请重启"),QMessageBox::Yes);
    mbox.setStyleSheet(
                "QPushButton {"
                "font:30px;"
                "padding-left:100px;"
                "padding-right:100px;"
                "padding-top:40px;"
                "padding-bottom:40px;"
                "}"
                "QLabel { font:30px;}"
                );
    mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
    mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
    mbox.exec();
}

void ControlCenter::onTaskFinish(int taskId)
{
    if(taskId == 0)return ;//返回原点的任务完成了,什么也不用做

    //1.找到完成的任务
    Task finishTask;
    foreach (auto t, doingTasks)
    {
        if(taskId == t.id){
            finishTask = t;
            break;
        }
    }
    if(finishTask.id <0)return ;//未找到该任务

    //2.修改任务、亮灯、界面状态
    if(finishTask.line == Task::LineA)
    {
        taskAFinish = true;
        rf.lightOff(RADOI_FREQUENCY_ADDRESS_A);
        centerWidget->finishA();
    }else{
        taskBFinish = true;
        rf.lightOff(RADOI_FREQUENCY_ADDRESS_B);
        centerWidget->finishB();
    }

    //3.移除完成的任务并保存数据库
    finishTask.task_finishTime = QDateTime::currentDateTime();
    doingTasks.removeOne(finishTask);
    QString updateSql = "update agv_task set task_finishTime=? where id=?";;
    QStringList params;
    params<<finishTask.task_excuteTime.toString(DATE_TIME_FORMAT)
         <<QString("%1").arg(finishTask.id);
    if(!g_sql->exeSql(updateSql,params))
    {
        QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("对任务数据库更新失败"),QMessageBox::Yes);
        mbox.setStyleSheet(
                    "QPushButton {"
                    "font:30px;"
                    "padding-left:100px;"
                    "padding-right:100px;"
                    "padding-top:40px;"
                    "padding-bottom:40px;"
                    "}"
                    "QLabel { font:30px;}"
                    );
        mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
        mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
        mbox.exec();
    }

    //4.找到执行车辆,为后续车辆接下来是返回原点还是执行下一个任务做准备
    AgvConnector *excuteAgv = NULL;
    foreach (auto agv, agvs)
    {
        if(agv->getId() == finishTask.excuteAgv)
        {
            excuteAgv = agv;
            break;
        }
    }
    if(excuteAgv == NULL)return ;//未能找到执行车辆

    //5.新增对下一个任务的判断：判断下一个任务是不是LineB。如果是，继续执行下一个任务，否则 回原点
    Task nextTask;
    foreach(auto tt, todoTasks) {
        int station;
        if(tt.line == Task::LineA){
            station = centerWidget->getNextAStation();
        }else{
            station = centerWidget->getNextBStation();
        }
        if(station != -1)
        {
            nextTask = tt;
            break;
        }
    }

    if(nextTask.id <0 || nextTask.line == Task::LineA){
        //5.0.没有下一个任务 或下一个任务是A任务
        excuteAgv->goOrigin();
    }else{
        //5.1.执行下一个任务
        int station;
        if(nextTask.line == Task::LineA){
            station = centerWidget->getNextAStation();
        }else{
            station = centerWidget->getNextBStation();
        }
        if(station == -1)
        {
            excuteAgv->goOrigin();//没有货物可取，那就返回原点
            return ;
        }

        //5.2.执行
        if(excuteAgv->sendTask(nextTask.id,nextTask.line,station,true))
        {
            //5.3.执行成功，设置任务状态
            nextTask.station = station;
            nextTask.excuteAgv = excuteAgv->getId();
            nextTask.task_excuteTime = QDateTime::currentDateTime();
            //5.4.更新任务执行时间/执行的车辆和站点
            QString updateSql = "update agv_task set station=?,excuteAgv=?,task_excuteTime=?  where id=?";;
            QStringList params;
            params<<QString("%1").arg(nextTask.station)
                 <<QString("%1").arg(nextTask.excuteAgv)
                <<nextTask.task_excuteTime.toString(DATE_TIME_FORMAT)
               <<QString("%1").arg(nextTask.id);
            if(!g_sql->exeSql(updateSql,params)){
                //更新数据库失败
                QMessageBox mbox(QMessageBox::NoIcon,QStringLiteral("错误"),QStringLiteral("对新任务存库失败"),QMessageBox::Yes);
                mbox.setStyleSheet(
                            "QPushButton {"
                            "font:30px;"
                            "padding-left:100px;"
                            "padding-right:100px;"
                            "padding-top:40px;"
                            "padding-bottom:40px;"
                            "}"
                            "QLabel { font:30px;}"
                            );
                mbox.setButtonText (QMessageBox::Yes,QStringLiteral("确 定"));
                mbox.setButtonText (QMessageBox::No,QStringLiteral("取 消"));
                mbox.exec();
            }
            doingTasks.append(nextTask);
            todoTasks.removeOne(nextTask);

            //5.5.将货物从UI界面中移除。
            if(nextTask.line == Task::LineA){
                centerWidget->takeGoodA();
            }
            else{
                centerWidget->takeGoodB();
            }
        }else{
            //执行失败，返回原点
            excuteAgv->goOrigin();
        }

    }
}
