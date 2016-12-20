#include "Common.h"
#include "GetReceptionCarNumber.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetReceptionCarNumbers, "get_reception_car_numbers")


using namespace auto_review;

GetReceptionCarNumbers::GetReceptionCarNumbers(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetReceptionCarNumbers::exec()
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
    autoNumbers << "к002ст23";
    autoNumbers << "в170ьь161";
    autoNumbers << "а853мр97";
    autoNumbers << "р070вк92";
    autoNumbers << "с065мк78";
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

void GetReceptionCarNumbers::setError(const QString& err)
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
