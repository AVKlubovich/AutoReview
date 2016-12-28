#include "Common.h"
#include "GetCarDamage.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetCarDamage, "get_car_damage")


using namespace auto_review;

GetCarDamage::GetCarDamage(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetCarDamage::exec()
{
    qDebug() << "get_car_damage";
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto incomingData = _context._packet.body().toMap();
    auto uData = incomingData.value("body").toMap();

    const auto id = uData["id_car"].toLongLong();
    const auto wraper = database::DBManager::instance().getDBWraper();
    auto selectQuery = wraper->query();

    const auto sqlQuery = QString("SELECT "
                                  "id,"
                                  "id_element_damage,"
                                  "type_damage,"
                                  "comment"
                                  " FROM car_damage"
                                  " WHERE id_car=:id"
                                  " AND status=false");
    selectQuery.prepare(sqlQuery);
    selectQuery.bindValue(":id", id);
    bool addCarQueryResult = wraper->execQuery(selectQuery);
    if (!addCarQueryResult)
    {
        sendError("error select car_damage", "error", signature());
        qDebug() << "error select car_damage" << selectQuery.lastError();
        return QSharedPointer<network::Response>();
    }

    const auto listCar = database::DBHelpers::queryToVariant(selectQuery);

    QVariantMap body, head, result;
    head["type"] = signature();
    body["status"] = 1;
    body["damages"] = listDamages(listCar);

    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

QVariantList GetCarDamage::listDamages(const QVariantList &list)
{
    QVariantList listResult;

    for (auto item : list)
    {
        auto map = item.toMap();
        const auto wraper = database::DBManager::instance().getDBWraper();
        auto selectQuery = wraper->query();

        const int id_damage = map["id"].toInt();
        const auto sqlQueryPhotos = QString("SELECT url"
                                            " FROM photos"
                                            " WHERE photos.id_damage=:id");
        selectQuery.prepare(sqlQueryPhotos);
        selectQuery.bindValue(":id", id_damage);
        bool addPhotosQueryResult = wraper->execQuery(selectQuery);

        QVariantList listPhotos;
        if (!addPhotosQueryResult)
            listPhotos.append("error select photos from id_damage = " + QString::number(id_damage));
        else
            listPhotos = database::DBHelpers::queryToVariant(selectQuery);

        QVariantMap mapObject = map;
        mapObject["photos"] = listPhotos;

        listResult.append(mapObject);
    }

    return listResult;
}
