#include "sql.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include "global.h"

Sql::Sql()
{

}

Sql::~Sql()
{
    closeConnection();
}

//两张表
//有可能只有一辆车，也可能是多辆车
//表1 车辆表 agv_agv[字段包括 id,ip,端口]
//表2 任务表 agv_task[字段包括 id, 创建时间 ,执行时间,完成时间,货物类型,呼叫站点, 执行车辆]
bool Sql::checkTables()
{
    //mysql
    QString querySql = "select count(*) from INFORMATION_SCHEMA.TABLES where TABLE_NAME=? ;";
    //sqlite
    //QString querySql = "SELECT COUNT(*) FROM sqlite_master where type='table' and name=?";

    QStringList args;

    args.clear();
    args<<"agv_task";
    QList<QStringList> qsl = query(querySql,args);
    if(qsl.length()!=1||qsl[0].length()!=1||qsl[0][0]=="0"){
        //不存在.创建
        QString createSql = "create table agv_task (id INTEGER PRIMARY KEY AUTO_INCREMENT,task_createTime datetime,task_excuteTime datetime,task_finishTime datetime,task_line INTEGER,station INTEGER,excuteAgv INTEGER);";
        args.clear();
        bool b = exeSql(createSql,args);
        if(!b)return false;
    }

    return true;
}



//创建数据库连接
bool Sql::createConnection()
{
    if (QSqlDatabase::contains("mysqliteconnection"))
    {
        database = QSqlDatabase::database("mysqliteconnection");
    }
    else
    {
        database = QSqlDatabase::addDatabase("QMYSQL","mysqliteconnection");
        database.setHostName(configure.getValue("mysql/host").toString());
        database.setDatabaseName(configure.getValue("mysql/database").toString());
        database.setPort(configure.getValue("mysql/port").toInt());

        database.setUserName(configure.getValue("mysql/username").toString());
        database.setPassword(configure.getValue("mysql/password").toString());
    }

    if(!database.isValid())return false;

    if (!database.open())
    {
        qDebug() << "Error: Failed to connect database."<<database.lastError();
        return false;
    }
    return checkTables();
}

//关闭数据库连接
bool Sql::closeConnection()
{
    database.close();
    return true;
}

//执行sql语句
bool Sql::exeSql(QString esql,QStringList args)
{
    qDebug() << "exesql="<<esql;
    mutex.lock();
    QSqlQuery sql_query(database);
    sql_query.prepare(esql);
    for(int i=0;i<args.length();++i){
        sql_query.addBindValue(args[i]);
    }

    if(!sql_query.exec())
    {
        qDebug() << "Error: Fail to sql_query.exec()."<<sql_query.lastError();
        mutex.unlock();
        return false;
    }
    mutex.unlock();

    return true;
}

//查询数据
QList<QStringList> Sql::query(QString qeurysql, QStringList args)
{
    QList<QStringList> xx;
    if(qeurysql.contains("@@Identity")){
        QString insertSql = qeurysql.split(";").at(0);
        QString querySqlNew = qeurysql.split(";").at(1);

        mutex.lock();
        QSqlQuery sql_insert(database);
        sql_insert.prepare(insertSql);
        for(int i=0;i<args.length();++i){
            sql_insert.addBindValue(args[i]);
        }

        if(!sql_insert.exec())
        {
            qDebug() << "Error: Fail to sql_query.exec()."<<sql_insert.lastError();
            mutex.unlock();
            return xx;
        }

        QSqlQuery sql_query(database);
        sql_query.prepare(querySqlNew);

        if(!sql_query.exec())
        {
            qDebug() << "Error: Fail to sql_query.exec()."<<sql_query.lastError();
            mutex.unlock();
            return xx;
        }
        while(sql_query.next()){
            int columnNum=sql_query.record().count();
            QStringList qsl;
            for(int i=0;i<columnNum;++i)
                qsl.append(sql_query.value(i).toString());
            xx.append(qsl);
        }
        mutex.unlock();
    }else{
        mutex.lock();
        QSqlQuery sql_query(database);
        sql_query.prepare(qeurysql);
        for(int i=0;i<args.length();++i){
            sql_query.addBindValue(args[i]);
        }
        if(!sql_query.exec())
        {
            qDebug() << "Error: Fail to sql_query.exec()."<<sql_query.lastError();
            mutex.unlock();
            return xx;
        }
        while(sql_query.next()){
            int columnNum=sql_query.record().count();
            QStringList qsl;
            for(int i=0;i<columnNum;++i)
                qsl.append(sql_query.value(i).toString());
            xx.append(qsl);
        }
        mutex.unlock();
    }
    return xx;
}


