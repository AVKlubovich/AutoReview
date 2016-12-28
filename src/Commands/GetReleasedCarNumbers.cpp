#include "Common.h"
#include "GetReleasedCarNumbers.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

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

    auto incomingData = _context._packet.body().toMap();
    auto bodyData = incomingData.value("body").toMap();

    const auto parkId = bodyData["id_park"].toInt();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    QVariantMap userData;
    userData["type_query"] = "get_autos_data";
    userData["park"] = QString::number(parkId);
    userData["our"] = QString::number(0);
    userData["user_login"] = bodyData.value("login");
    userData["user_pass"] = QString(QCryptographicHash::hash(bodyData.value("password").toString().toStdString().data(), QCryptographicHash::Md5).toHex());
    webRequest->setArguments(userData);
    webRequest->setCallback(nullptr);

    webManager->sendRequestCurrentThread(webRequest);

    const auto data = webRequest->reply();
    webRequest->release();

    const auto doc = QJsonDocument::fromJson(data);
    auto jobj = doc.object();
    const auto& map = jobj.toVariantMap();

    if (!map.contains("status"))
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: field not sended";
        return network::ResponseShp();
    }

    const auto status = map.value("status").toInt();
    if (status < 0)
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        return network::ResponseShp();
    }

    const auto& array = map["array"].toList();
    QVariantList numbersList;
    for (const auto& value : array)
    {
        QVariantMap valueMap = value.toMap();
        QVariantMap numberMap;
        numberMap.insert("id", valueMap["id"].toInt());
        numberMap.insert("number", valueMap["number"].toString());
        numbersList << QVariant::fromValue(numberMap);
    }

    QVariantMap result;

    QVariantMap head;
    head["type"] = signature();
    result["head"] = QVariant::fromValue(head);

    QVariantMap body;
    body["status"] = 1;
    body["cars"] = numbersList;
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}
