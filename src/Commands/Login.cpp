#include "Common.h"
#include "Login.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

#include "Definitions.h"

RegisterCommand(auto_review::Login, "login")


using namespace auto_review;

Login::Login(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp Login::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    auto webRequest = network::WebRequestShp::create( "sub_qry" );


    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("login") ||
        !bodyData.contains("password"))
    {
        setError(ERROR_LOGIN_OR_PASSWORD);
        network::ResponseShp();
    }

    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();

    QVariantMap userData;
    userData["sub_qry"] = "get_auto_review_rights";
    userData["user_login"] = userLogin;
    userData["user_pass"] = QString(QCryptographicHash::hash(userPass.toStdString().data(), QCryptographicHash::Md5).toHex());
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
        setError(ERROR_LOGIN_OR_PASSWORD);
        qDebug() << __FUNCTION__ << "error: field not sended";
        return network::ResponseShp();
    }

    const auto status = map.value("status").toInt();
    if (status < 0)
    {
        setError(ERROR_LOGIN_OR_PASSWORD, 2);
        return network::ResponseShp();
    }

    if (!map.contains("full_name") ||
        !map.contains("id"))
    {
        setError(ERROR_LOGIN_OR_PASSWORD);
        qDebug() << __FUNCTION__ << "error: field not sended";
        return network::ResponseShp();
    }

    bool basicRight = false;
    bool parkRight = false;
    qint64 idPark = -1;
    QString parkName;
    const auto& rightsArray = map["array"].toList();
    QVariantList availableStatuses;
    for (const auto& right : rightsArray)
    {
        const auto& rightMap = right.toMap();
        const auto idRight = rightMap["id_right"].toInt();

        if (idRight == BASIC_RIGHT)
        {
            basicRight = true;
        }
        else if (idRight == PARK_RIGHT)
        {
            parkRight = true;
            idPark = rightMap["id_park"].toULongLong();
            parkName = rightMap["name"].toString();
        }
        else if (idRight == AVAILABLE_STATUSES_RIGHT)
        {
            availableStatuses = rightMap["statuses"].toList();
        }
    }

    if (!basicRight || !parkRight || idPark == -1)
    {
        sendError(QObject::tr("Нет прав для работы в AutoReview"), "authotization_error", signature());
        return network::ResponseShp();
    }

    const auto& insertUserQueryStr = QString(
        "WITH upsert AS (UPDATE public.users SET name=:name1 WHERE id = :id1 RETURNING *)"
        "INSERT INTO public.users (id, name) SELECT :id2, :name2 WHERE NOT EXISTS (SELECT * FROM upsert)"
        );

    const quint64 userId = map.value("id").toULongLong();
    const QString& fullName = map.value("full_name").toString();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto insertUserQuery = wraper->query();
    insertUserQuery.prepare(insertUserQueryStr);
    insertUserQuery.bindValue(":id1", userId);
    insertUserQuery.bindValue(":name1", fullName);
    insertUserQuery.bindValue(":id2", userId);
    insertUserQuery.bindValue(":name2", fullName);

    auto insertUserQueryResult = wraper->execQuery(insertUserQuery);
    if (!insertUserQueryResult)
    {
        setError(ERROR_LOGIN_OR_PASSWORD);
        qDebug() << __FUNCTION__ << "error:" << qPrintable(insertUserQuery.lastError().text());
        return network::ResponseShp();
    }

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["id_user"] = userId;
    body["full_name"] = fullName;
    body["id_park"] = idPark;
    body["park_name"] = parkName;
    body["statuses"] = QVariant::fromValue(availableStatuses);

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}

void Login::setError(const QString& err, const quint64 status)
{
    QVariantMap body;
    QVariantMap head;
    QVariantMap result;
    head["type"] = signature();
    body["status"] = status;
    body["error"] = err;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));
}
