#include "Common.h"
#include "CheckDriver.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

#include "Definitions.h"

RegisterCommand(auto_review::CheckDriver, "check_driver")


using namespace auto_review;

CheckDriver::CheckDriver(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp CheckDriver::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("id_driver"))
    {
        sendError("Do not send field", "field_error", signature());
        return network::ResponseShp();
    }
    const auto driverId = bodyData["id_driver"].toLongLong();

    if (!bodyData.contains("pin"))
    {
        if (createDefaultPin(driverId))
        {
            sendError("Can not create drivers pin", "db_error", signature());
            qDebug() << __FUNCTION__ << "Can not create drivers pin";
            return network::ResponseShp();
        }
    }
    else
    {
        const auto pin = bodyData["pin"].toLongLong();

        const auto wraper = database::DBManager::instance().getDBWraper();

        const auto& selectPinStr = QString(
            "SELECT pin "
            "FROM drivers "
            "WHERE id = :driverId"
            );
        auto selectPinQuery = wraper->query();
        selectPinQuery.prepare(selectPinStr);
        selectPinQuery.bindValue(":driverId", driverId);

        const bool selectPinResult = selectPinQuery.exec();
        if (!selectPinResult)
        {
            sendError("Error select drivers pin", "db_error", signature());
            qDebug() << selectPinQuery.lastError().text();
            return network::ResponseShp();
        }

        const auto& resultList = database::DBHelpers::queryToVariantMap(selectPinQuery);
        if (resultList.isEmpty())
        {
            sendError("Drivers pin not found", "db_error", signature());
            qDebug() << __FUNCTION__ << "Drivers pin not found";
            return network::ResponseShp();
        }
        else
        {
            const qint64 dbPin = resultList.first()["pin"].toLongLong();

            if (dbPin != pin)
            {
                sendError("Drivers pin is not valid", "db_error", signature());
                qDebug() << __FUNCTION__ << "Drivers pin not found";
                return network::ResponseShp();
            }
        }
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

bool CheckDriver::createDefaultPin(const qint64 driverId)
{
    const auto wraper = database::DBManager::instance().getDBWraper();

    const auto& insertPinStr = QString(
        "INSERT INTO drivers "
        "(id, pin, date_last_change) "
        "VALUES "
        "(:driverId, :pin, now())"
        );
    auto insertPinQuery = wraper->query();
    insertPinQuery.prepare(insertPinStr);
    insertPinQuery.bindValue(":driverId", driverId);
    insertPinQuery.bindValue(":pin", DEFAULT_DRIVER_PIN);

    const bool insertPinResult = insertPinQuery.exec();
    if (!insertPinResult)
    {
        sendError("Error select drivers pin", "db_error", signature());
        qDebug() << insertPinQuery.lastError().text();
        return false;
    }

    return true;
}
