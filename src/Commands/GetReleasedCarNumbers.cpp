#include "Common.h"
#include "GetReleasedCarNumbers.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetReleasedCarNumber, "get_released_car_numbers")


using namespace auto_review;

GetReleasedCarNumber::GetReleasedCarNumber(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetReleasedCarNumber::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    QVariantMap body;
    QVariantMap head;
    QVariantMap result;

    head["type"] = signature();
    body["status"] = 1;
    result["head"] = QVariant::fromValue(head);
    QVariantList autoNumbers;
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 0}, {"number", "к002ст23"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 1}, {"number", "в170ьь161"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 2}, {"number", "а853мр97"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 3}, {"number", "р070вк92"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 4}, {"number", "с065мк78"}});
    body["cars"] = autoNumbers;
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

void GetReleasedCarNumber::setError(const QString& err)
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