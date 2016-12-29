#include "Common.h"
#include "SaveCarAccessories.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::SaveCarAccessories, "save_car_accessories")


using namespace auto_review;

SaveCarAccessories::SaveCarAccessories(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> SaveCarAccessories::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto incomingData = _context._packet.body().toMap();
    auto mapData = incomingData.value("body").toMap();

    const auto id = mapData["id_car"].toLongLong();
    auto listAccessories = mapData["accessories"].toList();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto updateQuery = wraper->query();

    for (const auto &element : listAccessories)
    {
        auto map = element.toMap();
        const auto sqlQuery = QString("UPDATE car_accessories SET status=:status, date_update=now(), comment=:comment "
                                      "WHERE id_car = :id AND id_accessory=:id_accessory");
        updateQuery.prepare(sqlQuery);
        updateQuery.bindValue(":id", id);
        updateQuery.bindValue(":status", map["status"]);
        updateQuery.bindValue(":comment", map["comment"]);
        updateQuery.bindValue(":id_accessory", map["id_accessory"]);
        bool addQueryResult = wraper->execQuery(updateQuery);

        if (!addQueryResult)
        {
            sendError("error insert car_accessories", "error", signature());
            qDebug() << "[ERROR]" << updateQuery.lastError();
            return QSharedPointer<network::Response>();
        }
    }

    QVariantMap body;
    QVariantMap head;
    QVariantMap result;
    head["type"] = signature();
    body["status"] = 1;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}
