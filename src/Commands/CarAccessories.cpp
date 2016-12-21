#include "Common.h"
#include "CarAccessories.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::CarAccessories, "select_car_accessories")


using namespace auto_review;

CarAccessories::CarAccessories(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> CarAccessories::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto incomingData = _context._packet.body().toMap();
    auto uData = incomingData.value("body").toMap();

    const auto id = uData["id"].toString();
    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    const auto sqlQuery = QString("SELECT * "
                                  " FROM car_accessories"
                                  " WHERE id_car=:id");
    addQuery.prepare(sqlQuery);
    addQuery.bindValue(":id", id);
    bool addCarQueryResult = wraper->execQuery(addQuery);

    if (!addCarQueryResult)
        setError("error select car_accessories");
    const auto listAccessories = database::DBHelpers::queryToVariant(addQuery);


    QVariantMap body, head, result;
    head["type"] = signature();
    body["status"] = 1;
    body["accessories"] = listAccessories;

    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

void CarAccessories::setError(const QString& err)
{
    QVariantMap body;
    QVariantMap head;
    QVariantMap result;
    head["type"] = signature();
    body["status"] = -1;
    body["error"] = err;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));
}
