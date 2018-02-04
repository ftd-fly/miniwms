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
            connect(agv,SIGNAL(cancel(int)),this,SLOT(onTaskFinish(int)));//TODO:取消的任务也任务是完成了？
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
        station = centerWidget->getNextAStation();
    }else{
        station = centerWidget->getNextBStation();
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
        todoTasks.removeAt(0);

        //开始执行，便将货物从UI界面中移除。
        if(t.line == Task::LineA){
            centerWidget->takeGoodA();
        }
        else{
            centerWidget->takeGoodB();
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
    for(int i=0;i<doingTasks.length();++i)
    {
        Task t = doingTasks.at(i);
        if(t.id == taskId)
        {
            if(t.line == Task::LineA)
            {
                taskAFinish = true;
                rf.lightOff(RADOI_FREQUENCY_ADDRESS_A);
            }else{
                taskBFinish = true;
                rf.lightOff(RADOI_FREQUENCY_ADDRESS_B);
            }
            t.task_finishTime = QDateTime::currentDateTime();

            //更新任务完成时间
            QString updateSql = "update agv_task set task_finishTime=? where id=?";;
            QStringList params;
            params<<t.task_excuteTime.toString(DATE_TIME_FORMAT)
                 <<QString("%1").arg(t.id);

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

            doingTasks.removeAt(i);
            break;
        }
    }
}
