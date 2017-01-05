#include "Common.h"
#include "SaveDamageCar.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::SaveDamageCar, "save_damage_car")


using namespace auto_review;

SaveDamageCar::SaveDamageCar(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp SaveDamageCar::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();

    const auto carId = incomingData["id_car"].toInt();
    const auto& damages = incomingData["damages"].toList();

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    for (const auto& damage : damages)
    {
        const auto& mapDamage = damage.toMap();
        const auto idElementDamage = mapDamage["id_element"].toInt();
        const auto typeDamage = mapDamage["id_damage"].toInt();
        const auto& comment = mapDamage["comment"].toString();
        const auto& listUrlsPhoto = mapDamage["urls"].toList();

        const auto& sqlQuery = QString(
            "INSERT INTO car_damage "
            "(id_car, id_element_damage, type_damage, comment, date_create) "
            "VALUES "
            "(:carId, :idElementDamage, :typeDamage, :comment, now()) "
            "RETURNING id");
        addQuery.prepare(sqlQuery);
        addQuery.bindValue(":carId", carId);
        addQuery.bindValue(":idElementDamage", idElementDamage);
        addQuery.bindValue(":typeDamage", typeDamage);
        addQuery.bindValue(":comment", comment);
        bool addPhotosQueryResult = wraper->execQuery(addQuery);

        if (!addPhotosQueryResult)
        {
            sendError("error insert car_damage", "error", signature());
            qDebug() << addQuery.lastError().text();
            return network::ResponseShp();
        }

        if (!addQuery.first())
        {
            sendError("error receipt id car_damage", "error", signature());
            return network::ResponseShp();
        }

        quint64 idCarDamage = addQuery.value("id").toLongLong();;

        for (const auto &url : listUrlsPhoto)
        {
            const auto& sqlQuery = QString(
                "INSERT INTO photos "
                "(id_car_damage, url) "
                "VALUES "
                "(:carId, :url)");
            addQuery.prepare(sqlQuery);
            addQuery.bindValue(":carId", idCarDamage);
            addQuery.bindValue(":url", url.toString());

            bool addPhotosQueryResult = wraper->execQuery(addQuery);
            if (!addPhotosQueryResult)
            {
                sendError("error insert photos", "error", signature());
                qDebug() << addQuery.lastError().text();
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
