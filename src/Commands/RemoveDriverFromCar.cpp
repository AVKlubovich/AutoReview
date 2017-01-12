#include "Common.h"
#include "RemoveDriverFromCar.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

RegisterCommand(auto_review::RemoveDriverFromCar, "remove_driver_from_car")


using namespace auto_review;

RemoveDriverFromCar::RemoveDriverFromCar(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp RemoveDriverFromCar::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("login") ||
        !bodyData.contains("password") ||
        !bodyData.contains("id_car"))
    {
        sendError("Do not send field", "field_error", signature());
        return network::ResponseShp();
    }

    const auto autoId = bodyData["id_car"].toInt();

    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("sub_qry");

    QVariantMap userData;
    userData["sub_qry"] = "autoreview_set_driver_to_auto";
    userData["user_login"] = userLogin;
    userData["user_pass"] = QString(QCryptographicHash::hash(userPass.toStdString().data(), QCryptographicHash::Md5).toHex());
    userData["auto_id"] = QString::number(autoId);
    userData["driver_id"] = "0";
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
        qDebug() << __FUNCTION__ << errorStr;
        return network::ResponseShp();
    }

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}
