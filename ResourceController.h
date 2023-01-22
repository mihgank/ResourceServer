#ifndef RESOURCECONTROLLER_H
#define RESOURCECONTROLLER_H

//TODO: ReserveResource должно отправлять оповещение пользователю, у которого забрали ресурс
//TODO: Конфиг бы с Ini файла подгружать
#include <algorithm>
#include <mutex>

#include <QVector>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>

#include "utils.h"

using namespace std;


class TConfig {
public:
    TConfig() {
        _usersList = {
                "vasa",
                "peta",
                "roma",
                "glasha",
                "vanya",
        };

        _resourcesCount = 4;
        _lifeTimeHours = 2;
    }

    unsigned ResourcesCount() const {
        return _resourcesCount;
    }

    const QVector<QString>& UsersList() const {
        return  _usersList;
    }

    quint64 LifeTime() const{
         return _lifeTimeHours * 60 * 60;

    }


private:
    QVector<QString> _usersList;
    unsigned _resourcesCount;
    unsigned _lifeTimeHours;
};

enum class EResourceStatus {
    Free,
    Busy
};

class TResource {

};

struct TResourceData {
    EResourceStatus _satus = EResourceStatus::Free;
    quint64 _t_start = 0;
    quint64 _t_take = 0;
    QString _reservedByUser;


    TResource _resource;
};


class TResourceController final {
public:
    TResourceController(const TConfig& config)
            : _config{config}
    {
        _resourcesData.resize(config.ResourcesCount());
    }

    bool CheckUserAccess(const QString& userName) {

        const auto& usersList = _config.UsersList();

        auto it = std::find_if(usersList.begin(), usersList.end(), [userName](const auto& value){
            return value == userName;
        });

        return it !=  usersList.end();

    }

    bool ReserveResource(const QString& userName, unsigned idx, quint64 time) {
        std::lock_guard<std::mutex> guard(_resources_mutex);
        if(idx >= _resourcesData.size()) {
            qDebug() << "Недопустимый ресурс";
            return false;
        }

        if( !CheckUserAccess(userName) ) {
            qDebug() << "У пользователя: " << userName << " нет доступа";
            return false;
        }

        auto& data = _resourcesData[idx];

        auto currentTimestamp = utils::GetCurrentTimestamp();

        //если пользователь занимал ресурс менее чем на 'Z' часов
        if(data._satus == EResourceStatus::Busy){
            if(data._t_start + data._t_take > currentTimestamp){
                data._t_start = currentTimestamp;
                data._t_take = time;
                data._reservedByUser = userName;
                return true;
            }
        }

        //пользователь продлевает резервацию ресурса
        if(data._satus == EResourceStatus::Busy && data._reservedByUser == userName ) {
            data._t_start = currentTimestamp;
            data._t_take = time;
            return true;
        }

        if(data._satus != EResourceStatus::Free) {
            if(data._t_start + _config.LifeTime() > currentTimestamp) {
                qDebug() << "Ресурс еще занят";
                return false;
            } else {
                qDebug() << "Ресурс освобожден";
                return true;
            }
            //шлем сообщение клиенту data._reservedByUser что его ресурс освобожден
            //тут надо как-то SendMessage вызвать
        }

        data._t_start = currentTimestamp;
        data._t_take = time;
        data._reservedByUser = userName;
        data._satus = EResourceStatus::Busy;
        qDebug() << "Успех!\n" << userName << ", Вы получили доступ к ресурсу";
        return true;
    }

    TResourceData* GetClientResource(const QString& userName) {
        std::lock_guard<std::mutex> guard(_resources_mutex);

        auto it = std::find_if(_resourcesData.begin(), _resourcesData.end(), [userName](const auto& value){
            return value._satus == EResourceStatus::Busy && value._reservedByUser == userName;
        });

        return it != _resourcesData.end() ? &*it : nullptr;
    }

private:
    std::mutex _resources_mutex;
    QVector<TResourceData> _resourcesData;
    const TConfig& _config;
};

#endif // RESOURCECONTROLLER_H
