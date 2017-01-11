#include "Common.h"
#include "SaveCarAccessories.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::SaveCarAccessories, "save_car_accessories")


using namespace auto_review;

SaveCarAccessories::SaveCarAccessories(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp SaveCarAccessories::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& mapData = incomingData.value("body").toMap();

    const auto carId = mapData["id_car"].toLongLong();
    const auto& listAccessories = mapData["accessories"].toList();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto updateQuery = wraper->query();

    for (const auto& element : listAccessories)
    {
        const auto& map = element.toMap();
        const auto status = map["status"].toInt();
        const auto& comment = map["comment"].toString();
        const auto accessoryId = map["id_accessory"].toInt();

        const auto& sqlQuery = QString(
            "UPDATE car_accessories "
            "SET "
            "status = :status, "
            "date_update = now(), "
            "comment = :comment "
            "WHERE id_car = :carId AND id_accessory = :accessoryId");
        updateQuery.prepare(sqlQuery);
        updateQuery.bindValue(":carId", carId);
        updateQuery.bindValue(":status", status);
        updateQuery.bindValue(":comment", comment);
        updateQuery.bindValue(":accessoryId", accessoryId);

        bool addQueryResult = wraper->execQuery(updateQuery);
        if (!addQueryResult)
        {
            sendError("error insert car_accessories", "db_error", signature());
            qDebug() << updateQuery.lastError().text();
            return network::ResponseShp();
        }
    }

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}
