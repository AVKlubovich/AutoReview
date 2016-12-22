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

    const auto& incomingData = _context._packet.body().toMap()["body"].toMap();

    if (!incomingData.contains("id_park"))
    {
        sendError("", "incoming_data_error", signature());
        return network::ResponseShp();
    }

    auto webManager = network::WebRequestManager::instance();

//    QVariantMap userData;
//    userData["sub_qry"] = "get_auto_review_rights";
//    userData["user_login"] = uData.value("login");
//    userData["user_pass"] = QString(QCryptographicHash::hash(uData.value("password").toString().toStdString().data(), QCryptographicHash::Md5).toHex());
//    webRequest->setArguments(userData);
//    webRequest->setCallback(nullptr);

//    webManager->sendRequestCurrentThread(webRequest);

//    const auto data = webRequest->reply();
//    webRequest->release();

//    const auto doc = QJsonDocument::fromJson(data);
//    auto jobj = doc.object();
//    const auto map = jobj.toVariantMap();

//    if(!map.contains("status"))
//    {
//        // TODO: db_error
//        setError(ERROR_LOGIN_OR_PASSWORD);
//        qDebug() << __FUNCTION__ << "error: field not sended";
//        return QSharedPointer<network::Response>();
//    }

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
