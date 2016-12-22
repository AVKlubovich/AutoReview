#include "Common.h"
#include "GetAcceptedCarNumbers.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetAcceptedCarNumbers, "get_accepted_car_numbers")


using namespace auto_review;

GetAcceptedCarNumbers::GetAcceptedCarNumbers(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetAcceptedCarNumbers::exec()
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
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 0}, {"number", "в104ту178"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 0}, {"number", "к002ст23"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 0}, {"number", "кк002ст23"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 1}, {"number", "в170ьь161"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 1}, {"number", "в170ьи161"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 2}, {"number", "а853мр97"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 3}, {"number", "р070вк92"}});
    autoNumbers << QVariant::fromValue(QVariantMap() = {{"id", 4}, {"number", "с065мк78"}});
    body["cars"] = autoNumbers;
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}
