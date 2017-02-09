#include "Common.h"
#include "WSGetStartData.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

RegisterCommand(auto_review::WSGetStartData, "ws_get_start_data")


using namespace auto_review;

WSGetStartData::WSGetStartData(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp WSGetStartData::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& wraper = database::DBManager::instance().getDBWraper();

    const auto& selectCarsStr = QString(
        "SELECT "
        "info_about_cars.id, "
        "id_car, "
        "info_about_cars.mileage, "
        "insurance_end, "
        "diagnostic_card_end, "
        "id_tire, "
        "(SELECT maintenance.mileage FROM maintenance WHERE id_car = info_about_cars.id_car AND maintenance.is_big = 0 ORDER BY id DESC LIMIT 1) as last_maintenance, "
        "(SELECT maintenance.mileage FROM maintenance WHERE id_car = info_about_cars.id_car AND maintenance.is_big = 1 ORDER BY id DESC LIMIT 1) as last_big_maintenance, "
        "date_create AS last_timestamp, "
        "osago_date, "
        "osago_number, "
        "pts, "
        "srts, "
        "tire_marka, "
        "diagnostic_card_data, "
        "child_restraint_means "
        "FROM info_about_cars "
        "INNER JOIN maintenance "
        "ON (maintenance.id_car = info_about_cars.id_car) "
//        "WHERE info_about_cars.status = :status"
        );

    auto selectCarsQuery = wraper->query();
    selectCarsQuery.prepare(selectCarsStr);
//    selectCarsQuery.bindValue(":status", status_in_workshop);

    const auto selectCarsResult = selectCarsQuery.exec();
    if (!selectCarsResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << selectCarsQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectCarsQuery.lastQuery();
        return network::ResponseShp();
    }

    auto carsList = database::DBHelpers::queryToVariant(selectCarsQuery);
    QStringList carIdsList;
    for (const auto& carInfo : carsList)
    {
        const QString& carId = carInfo.toMap()["id_car"].toString();
        carIdsList << carId;
    }

    const auto& selectDamagesStr = QString(
        "SELECT * FROM car_damage"
        "WHERE id_car IN (%1)"
        ).arg(carIdsList.join(","));

    auto selectDamagesQuery = wraper->query();
    selectDamagesQuery.prepare(selectDamagesStr);

    const auto selectDamagesResult = selectDamagesQuery.exec();
    if (!selectDamagesResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << selectDamagesQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectDamagesQuery.lastQuery();
        return network::ResponseShp();
    }

    const auto& damages = database::DBHelpers::queryToVariant(selectDamagesQuery);

    QMap<quint64, QVariantMap> carsInfoMap;
    for (auto& carInfo : carsList)
    {
        QVariantMap& carInfoMap = carInfo.toMap();
        const quint64 carId = carInfoMap["id_car"].toULongLong();
        carsInfoMap.insert(carId, carInfoMap);
    }

    for (const auto& damage : damages)
    {
        const QVariantMap& damageMap = damage.toMap();
        const quint64 carId = damageMap["id_car"].toULongLong();
        QVariantMap& carInfoMap = carsInfoMap[carId];
        QVariantList& damageList = carInfoMap["damages"].toList();
        damageList << damageMap;
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
