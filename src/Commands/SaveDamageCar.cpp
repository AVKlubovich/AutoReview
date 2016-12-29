#include "Common.h"
#include "SaveDamageCar.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::SaveDamageCar, "save_damage_car")


using namespace auto_review;

SaveDamageCar::SaveDamageCar(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> SaveDamageCar::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto incomingData = _context._packet.body().toMap();
    auto uData = incomingData.value("body").toMap();

    const auto id = uData["id_car"].toLongLong();
    const auto id_element_damage = uData["id_element"].toInt();
    const auto type_damage = uData["id_damage"].toInt();
    const auto comment = uData["comment"].toString();
    auto listUrlsPhoto = uData["urls"].toList();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    const auto sqlQuery = QString("INSERT INTO car_damage (id_car, id_element_damage, type_damage, comment, date_create)"
                                  "VALUES (:id, :id_element_damage, :type_damage, :comment, now()) RETURNING id");
    addQuery.prepare(sqlQuery);
    addQuery.bindValue(":id", id);
    addQuery.bindValue(":id_element_damage", id_element_damage);
    addQuery.bindValue(":type_damage", type_damage);
    addQuery.bindValue(":comment", comment);
    bool addPhotosQueryResult = wraper->execQuery(addQuery);

    if (!addPhotosQueryResult)
    {
        sendError("error insert car_damage", "error", signature());
        qDebug() << "[ERROR]" << addQuery.lastError();
        return QSharedPointer<network::Response>();
    }
    const auto idCarDamage = addQuery.value("id").toLongLong();

    for (auto url : listUrlsPhoto)
    {
        const auto sqlQuery = QString("INSERT INTO photos (id_car_damage, url) VALUES (:id, :url)");
        addQuery.prepare(sqlQuery);
        addQuery.bindValue(":id", idCarDamage);
        addQuery.bindValue(":url", url.toString());
        bool addPhotosQueryResult = wraper->execQuery(addQuery);

        if (!addPhotosQueryResult)
        {
            sendError("error insert photos", "error", signature());
            return QSharedPointer<network::Response>();
        }
    }




    QVariantMap body, head, result;
    head["type"] = signature();
    body["status"] = 1;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}
