#include "Common.h"
#include "GetDrivers.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

RegisterCommand(auto_review::GetDrivers, "get_drivers")


using namespace auto_review;

GetDrivers::GetDrivers(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetDrivers::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("login") ||
        !bodyData.contains("password") ||
        !bodyData.contains("id_park") ||
        !bodyData.contains("search"))
    {
        sendError("Do not send field", "field_error", signature());
        return network::ResponseShp();
    }

    const auto& searchStr = bodyData["search"].toString();
    if (searchStr.count() < 3)
    {
        sendError("Search lenght less than 3", "bad_parameter", signature());
        return network::ResponseShp();
    }

    const auto parkId = bodyData["id_park"].toInt();
    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("sub_qry");

    QVariantMap userData;
    userData["sub_qry"] = "autoreview_driver_search";
    userData["user_login"] = userLogin;
    userData["user_pass"] = QString(QCryptographicHash::hash(userPass.toStdString().data(), QCryptographicHash::Md5).toHex());
    userData["our"] = "0";
    userData["park"] = QString::number(parkId);
    userData["search"] = searchStr;
    webRequest->setArguments(userData);
    webRequest->setCallback(nullptr);

    webManager->sendRequestCurrentThread(webRequest);

    const auto& data = webRequest->reply();
    webRequest->release();

    const auto& doc = QJsonDocument::fromJson(data);
    const auto& jobj = doc.object();
    const auto& map = jobj.toVariantMap();

    if (!map.contains("status"))
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: field not sended";
        return network::ResponseShp();
    }

    const auto status = map["status"].toInt();
    if (status != 1)
    {
        const auto& errorList = map["error"].toList();
        const auto& errorStr = errorList.first().toString();
        sendError(errorStr, "remove_server_error", signature());
        return network::ResponseShp();
    }

    const auto& driversArray = map["array"].toList();

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["drivers"] = QVariant::fromValue(driversArray);

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}
