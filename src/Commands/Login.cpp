#include "Common.h"
#include "Login.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "permissions/PermissionManager.h"

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
    qDebug() << __FUNCTION__ << " was runned" << QDateTime::currentDateTime();

    auto & responce = _context._responce;
    responce->setHeaders(_context._packet.headers());

    //decode incoming data
    auto incomingData = _context._packet.body();
    QSharedPointer<network::RequestLogin> requestLoginPtr(new network::RequestLogin(incomingData));
    requestLoginPtr->fromVariant(incomingData);

    //create command response
    QSharedPointer<network::ResponseLogin> response(new network::ResponseLogin(requestLoginPtr));

    const auto login = requestLoginPtr->login();
    const auto password = requestLoginPtr->password();
    quint64 idUser;

    QSharedPointer<database::DBWraper> wraper = database::DBManager::instance().getDBWraper();
    QSqlQuery query = wraper->query();

    // find id user
    QString sqlQuery= QString("SELECT id FROM users_schema.users "
                              "WHERE login = '%1' AND password = '%2' AND \"isVisible\" = 0")
                      .arg(login).arg(password);
    query.prepare(sqlQuery);

    const auto queryResult = wraper->execQuery(query);
    if (!queryResult)
    {
        qDebug() << query.lastError();
        response->setError(query.lastError().text());
        response->setStatus(network::ResponseLogin::StatusError);
        return response;
    }

    if (query.size())
    {
        query.first();
        idUser = query.value("id").toLongLong();

        sqlQuery= QString("SELECT * FROM users_schema.permissions WHERE id_user = %1")
                  .arg(idUser);
        query.prepare(sqlQuery);

        const auto queryResult = wraper->execQuery(query);
        if (!queryResult)
        {
            qDebug() << query.lastError();
            response->setError(query.lastError().text());
            response->setStatus(network::ResponseLogin::StatusError);
            return response;
        }

        const auto resultList = database::DBHelpers::queryToVariant(query);
        response->setUserRights(resultList);

        response->setStatus(network::ResponseLogin::StatusSuccess);
    }
    else
    {
        response->setStatus(network::ResponseLogin::StatusRejected);
    }

    return response;
}
