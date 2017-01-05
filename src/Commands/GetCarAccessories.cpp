#include "Common.h"
#include "GetCarAccessories.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetCarAccessories, "get_car_accessories")


using namespace auto_review;

GetCarAccessories::GetCarAccessories(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetCarAccessories::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& mapData = incomingData.value("body").toMap();

    const auto& id = mapData["id_car"].toString();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    const auto& sqlQuery = QString(
        "SELECT id_accessory AS id, status, comment, date_update "
        "FROM car_accessories "
        "WHERE id_car=:id");
    addQuery.prepare(sqlQuery);
    addQuery.bindValue(":id", id);
    bool addCarQueryResult = wraper->execQuery(addQuery);

    if (!addCarQueryResult)
    {
        sendError("error select car_accessories", "error", signature());
        qDebug() << addQuery.lastError().text();
        return network::ResponseShp();
    }

    const auto& listAccessories = database::DBHelpers::queryToVariant(addQuery);

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["accessories"] = listAccessories;

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}
