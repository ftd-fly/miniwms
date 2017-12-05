#include "configure.h"
#include <QSettings>
#include <QDebug>
Configure::Configure(): config_file("./config.conf")
{

}

void Configure::setConfigFile(const QString &file)
{
    config_file = file;
}

void Configure::load()
{
    config_map.clear();
    QSettings settings(config_file, QSettings::IniFormat);
    QStringList keys = settings.allKeys();
    for(QString key : keys)
    {
        config_map.insert(key,settings.value(key));
    }

    //这里有两种方案。
    //方案1.这里判定有些设置是否存在，如果不存在，那么在添加默认值.
    //方案2.在getvalue调用后，判断isvalid。然后给默认值.

    //这里讲采用方案1，其他处的代码会规整很多
    if(!config_map.contains("solution")){
        //方案A[36个货物存放点]
        //方案B[48个货物存放点]
        //方案C[60个货物存放点]
        config_map.insert("solution","C");
    }


}

void Configure::save()
{
    QSettings settings(config_file, QSettings::IniFormat);
    for (QMap<QString, QVariant>::iterator it = config_map.begin(); it != config_map.end(); ++it)
    {
        settings.setValue(it.key(), it.value());
    }
}

QVariant Configure::getValue(const QString &what)
{
    return config_map.value(what, QVariant());
}

void Configure::setValue(const QString &what, const QVariant &value)
{
    config_map.insert(what, value);
}
