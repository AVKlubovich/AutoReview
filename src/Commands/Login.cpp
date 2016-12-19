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

RegisterCommand(auto_review::Login, "login")


using namespace auto_review;

Login::Login(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> Login::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto webRequest = QSharedPointer<network::WebRequest>::create( "sub_qry" );


    auto incomingData = _context._packet.body().toMap();
    auto uData = incomingData.value("body").toMap();

    if(!uData.contains("login") ||
       !uData.contains("password"))
    {
        setError(ERROR_LOGIN_OR_PASSWORD);
        QSharedPointer<network::Response>();
    }

    auto webManager = network::WebRequestManager::instance();

    QVariantMap userData;
    userData["sub_qry"] = "get_auto_photo_validator_rights";
    userData["user_login"] = uData.value("login");
    userData["user_pass"] = QString(QCryptographicHash::hash(uData.value("password").toString().toStdString().data(),QCryptographicHash::Md5).toHex());
    webRequest->setArguments(userData);
    webRequest->setCallback(nullptr);

    webManager->sendRequestCurrentThread(webRequest);

    const auto data = webRequest->reply();
    webRequest->release();

    const auto doc = QJsonDocument::fromJson(data);
    auto jobj = doc.object();
    const auto map = jobj.toVariantMap();

    if(!map.contains("status"))
    {
        // TODO: db_error
        qDebug() << __FUNCTION__ << "error: field not sended";
        //Q_ASSERT(false);
        return QSharedPointer<network::Response>();
    }

    const auto status = map.value("status").toInt();
    if (status < 0)
    {
        setError(ERROR_LOGIN_OR_PASSWORD);
        return QSharedPointer<network::Response>();
    }

    if (!map.contains("full_name") ||
        !map.contains("id"))
    {
        // TODO: db_error
        qDebug() << __FUNCTION__ << "error: field not sended";
        //Q_ASSERT(false);
        return QSharedPointer<network::Response>();
    }

    bool rightOk = false;
    const auto& rightsArray = map["array"].toList();
    for (const auto& right : rightsArray)
    {
        const auto& rightMap = right.toMap();
        if (rightMap["id_right"].toInt() == 1)
        {
            rightOk = true;
            continue;
        }
    }

    if (!rightOk)
    {
        setError(QObject::tr("Not have permission"));
        return QSharedPointer<network::Response>();
    }

    const auto insertUserQueryStr = QString(
        "WITH upsert AS (UPDATE public.users SET name=:name1 WHERE id=:id1 RETURNING *)"
        "INSERT INTO public.users (id, name) SELECT :id2, :name2 WHERE NOT EXISTS (SELECT * FROM upsert)"
        );

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto insertUserQuery = wraper->query();
    insertUserQuery.prepare(insertUserQueryStr);
    insertUserQuery.bindValue(":id1", map.value("id"));
    insertUserQuery.bindValue(":name1", map.value("full_name"));
    insertUserQuery.bindValue(":id2", map.value("id"));
    insertUserQuery.bindValue(":name2", map.value("full_name"));

    auto insertUserQueryResult = wraper->execQuery(insertUserQuery);
    if (!insertUserQueryResult)
    {
        // TODO: db_error
        qDebug() << __FUNCTION__ << "error:" << qPrintable(insertUserQuery.lastError().text());
        //Q_ASSERT(false);
        return QSharedPointer<network::Response>();
    }

    QVariantMap body, head, result;
    QList <QVariant> listRights;
    listRights.append(map["user_rights"]);
    head["type"] = signature();
    body["status"] = 1;
    body["id_user"] = map["id"].toULongLong();
    body["user_rights"] = listRights;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

void Login::setError(const QString& err)
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
