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

    auto incomingData = _context._packet.body().toMap();
    auto bodyData = incomingData.value("body").toMap();

    const auto parkId = bodyData["id_park"].toInt();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("sub_qry");

    QVariantMap userData;
    userData["sub_qry"] = "get_autos_data";
    userData["park"] = parkId;
    userData["our"] = 0;
    userData["user_login"] = bodyData.value("login");
    userData["user_pass"] = QString(QCryptographicHash::hash(bodyData.value("password").toString().toStdString().data(), QCryptographicHash::Md5).toHex());
    webRequest->setArguments(userData);
    webRequest->setCallback(nullptr);

    webManager->sendRequestCurrentThread(webRequest);

    const auto data = webRequest->reply();
    webRequest->release();

    const auto doc = QJsonDocument::fromJson(data);
    auto jobj = doc.object();
    const auto map = jobj.toVariantMap();

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
