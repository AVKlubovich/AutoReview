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
    auto uData = incomingData.value("body").toMap();

    const auto id = uData["id"].toLongLong();
    auto listAccessories = uData["accessories"].toList();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    for (auto id_element : listAccessories)
    {
        const auto sqlQuery = QString("INSERT INTO car_accessories (id_car, id_accessories) VALUES (:id, :accessories)");
        addQuery.prepare(sqlQuery);
        addQuery.bindValue(":id", id);
        addQuery.bindValue(":accessories", id_element);
        bool addQueryResult = wraper->execQuery(addQuery);

        if (!addQueryResult)
        {
            sendError("error insert car_accessories", "error", signature());
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
