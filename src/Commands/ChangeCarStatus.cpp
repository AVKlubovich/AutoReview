#include "Common.h"
#include "ChangeCarStatus.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

RegisterCommand(auto_review::ChangeCarStatus, "change_car_status")


using namespace auto_review;

ChangeCarStatus::ChangeCarStatus(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp ChangeCarStatus::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    const auto autoId = bodyData["id_car"].toInt();
    const auto statusAuto = bodyData["status_car"].toInt();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    QVariantMap userData;
    userData["type_query"] = "change_auto_status";
    userData["user_login"] = bodyData.value("login").toString();
    userData["auto_id"] = QString::number(autoId);
    userData["status"] = QString::number(statusAuto);
    userData["user_pass"] = QString(QCryptographicHash::hash(bodyData.value("password").toString().toStdString().data(), QCryptographicHash::Md5).toHex());
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

    const auto status = map.value("status").toInt();
    if (status < 0)
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << map["err"].toString();
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

    return QSharedPointer<network::Response>();
}