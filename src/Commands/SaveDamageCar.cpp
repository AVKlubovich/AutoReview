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
    _wraper = database::DBManager::instance().getDBWraper();
}

network::ResponseShp SaveDamageCar::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();

    const auto carId = incomingData["id_car"].toInt();
    const auto& damages = incomingData["damages"].toList();

    _wraper->startTransaction();

    for (const auto& damage : damages)
    {
        qint64 damageId = -1;

        const auto& mapDamage = damage.toMap();
        if (mapDamage.contains("id_damage"))
        {
            damageId = updateDamage(mapDamage);
        }
        else
        {
            damageId = insertDamage(carId, mapDamage);
        }

        if (damageId == -1)
        {
            _wraper->rollback();
            return network::ResponseShp();
        }

        const auto& listUrlsPhoto = mapDamage["urls"].toList();

        auto insertPhotoQuery = _wraper->query();

        for (const auto &url : listUrlsPhoto)
        {
            const auto& insertPhotoStr = QString(
                "INSERT INTO photos "
                "(id_car_damage, url) "
                "VALUES "
                "(:carId, :url)");
            insertPhotoQuery.prepare(insertPhotoStr);
            insertPhotoQuery.bindValue(":carId", damageId);
            insertPhotoQuery.bindValue(":url", url.toString());

            bool insertPhotoResult = insertPhotoQuery.exec();
            if (!insertPhotoResult)
            {
                sendError("error insert photos", "error", signature());
                _wraper->rollback();
                qDebug() << insertPhotoQuery.lastError().text();
                return network::ResponseShp();
            }
        }
    }

    const bool commitOk = _wraper->commit();
    if (!commitOk)
    {
        _wraper->rollback();
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

const qint64 SaveDamageCar::updateDamage(const QVariantMap& damageMap)
{
    // NOTE: на случай, если понадобится изменить повреждение
    const auto idDamage = damageMap["id_damage"].toInt();
    return idDamage;
}

const qint64 SaveDamageCar::insertDamage(const int carId, const QVariantMap& damageMap)
{
    auto insertDamageQuery = _wraper->query();

    const auto idElementDamage = damageMap["id_element"].toInt();
    const auto typeDamage = damageMap["type_damage"].toInt();
    const auto& comment = damageMap["comment"].toString();

    const auto& insertDamageStr = QString(
        "INSERT INTO car_damage "
        "(id_car, id_element_damage, type_damage, comment, date_create) "
        "VALUES "
        "(:carId, :idElementDamage, :typeDamage, :comment, now()) "
        "RETURNING id");
    insertDamageQuery.prepare(insertDamageStr);
    insertDamageQuery.bindValue(":carId", carId);
    insertDamageQuery.bindValue(":idElementDamage", idElementDamage);
    insertDamageQuery.bindValue(":typeDamage", typeDamage);
    insertDamageQuery.bindValue(":comment", comment);

    const bool insertDamageResult = insertDamageQuery.exec();
    if (!insertDamageResult)
    {
        sendError("error insert car_damage", "error", signature());
        qDebug() << insertDamageQuery.lastError().text();
        return -1;
    }

    if (!insertDamageQuery.first())
    {
        sendError("error receipt id car_damage", "error", signature());
        return -1;
    }

    const qint64 idCarDamage = insertDamageQuery.value("id").toLongLong();
    return idCarDamage;
}
