#include "Common.h"
#include "GetCarDamage.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetCarDamage, "get_car_damage")


using namespace auto_review;

GetCarDamage::GetCarDamage(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetCarDamage::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& mapData = incomingData.value("body").toMap();

    const auto autoId = mapData["id_car"].toLongLong();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto selectQuery = wraper->query();

    const auto& sqlQuery = QString(
        "SELECT "
        "id, "
        "id_element_damage, "
        "type_damage, "
        "comment "
        "FROM car_damage"
        "WHERE id_car = :id AND status = false");
    selectQuery.prepare(sqlQuery);
    selectQuery.bindValue(":id", autoId);

    bool addCarQueryResult = wraper->execQuery(selectQuery);
    if (!addCarQueryResult)
    {
        sendError("error select car_damage", "db_error", signature());
        qDebug() << "error select car_damage" << selectQuery.lastError().text();
        return QSharedPointer<network::Response>();
    }

    const auto& listCar = database::DBHelpers::queryToVariant(selectQuery);

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["damages"] = listDamages(listCar);

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

QVariantList GetCarDamage::listDamages(const QVariantList &list)
{
    QVariantList listResult;

    for (const auto& item : list)
    {
        auto map = item.toMap();

        const auto wraper = database::DBManager::instance().getDBWraper();
        auto selectQuery = wraper->query();

        const int damageId = map["id"].toInt();
        const auto& sqlQueryPhotos = QString(
            "SELECT url "
            "FROM photos "
            "WHERE photos.id_car_damage = :damageId");
        selectQuery.prepare(sqlQueryPhotos);
        selectQuery.bindValue(":damageId", damageId);
        bool addPhotosQueryResult = wraper->execQuery(selectQuery);

        QVariantList listPhotos;
        if (!addPhotosQueryResult)
            listPhotos.append(QString("error select photos from id_car_damage = %1").arg(QString::number(damageId)));
        else
            listPhotos = database::DBHelpers::queryToVariant(selectQuery);

        map["photos"] = listPhotos;

        listResult.append(map);
    }

    return listResult;
}
