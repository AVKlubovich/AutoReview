#include "Common.h"
#include "SaveCarInfo.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::SaveCarInfo, "save_car_info")


using namespace auto_review;

SaveCarInfo::SaveCarInfo(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp SaveCarInfo::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    bool upsertResult = false;
    if (bodyData.contains("id_record"))
    {
        _recordId = bodyData["id_record"].toULongLong();
        upsertResult = updateNewData(bodyData);
    }
    else
    {
        upsertResult = insertNewData(bodyData);
    }

    if (!upsertResult)
    {
        sendError("Database error", "db_error", signature());
        return network::ResponseShp();
    }

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    if (_recordId != 0)
        body["id_record"] = _recordId;

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}

bool SaveCarInfo::insertNewData(const QVariantMap& bodyData)
{
    const auto carId = bodyData["id_car"].toLongLong();
    const auto mileage = bodyData["mileage"].toULongLong();
    const auto& insuranceEnd = bodyData["insurance_end"].toString();
    const auto& diagnosticCardEnd = bodyData["diagnostic_card_end"].toString();
    const auto tireId = bodyData["id_tire"].toInt();

    const auto& saveCarInfoStr = QString(
        "INSERT INTO info_about_cars "
        "("
            "id_car, "
            "mileage, "
            "insurance_end, "
            "diagnostic_card_end, "
            "id_tire, "
            "date_create"
        ") "
        "VALUES"
        "("
            ":carId, "
            ":mileage, "
            ":insuranceEnd, "
            ":diagnosticCardEnd, "
            ":tireId, "
            "now()"
        ") "
        "RETURNING id"
        );

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto saveCarInfoQuery = wraper->query();
    saveCarInfoQuery.prepare(saveCarInfoStr);
    saveCarInfoQuery.bindValue(":carId", carId);
    saveCarInfoQuery.bindValue(":mileage", mileage);
    saveCarInfoQuery.bindValue(":insuranceEnd", insuranceEnd);
    saveCarInfoQuery.bindValue(":diagnosticCardEnd", diagnosticCardEnd);
    saveCarInfoQuery.bindValue(":tireId", tireId);

    const auto saveCarInfoResult = saveCarInfoQuery.exec();
    if (!saveCarInfoResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastError().text();
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastQuery();
        return false;
    }

    if (!saveCarInfoQuery.first())
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastError().text();
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastQuery();
        return false;
    }

    _recordId = saveCarInfoQuery.value("id").toULongLong();

    return true;
}

bool SaveCarInfo::updateNewData(const QVariantMap& bodyData)
{
    const auto mileage = bodyData["mileage"].toULongLong();
    const auto& insuranceEnd = bodyData["insurance_end"].toString();
    const auto& diagnosticCardEnd = bodyData["diagnostic_card_end"].toString();
    const auto tireId = bodyData["id_tire"].toInt();

    const auto& saveCarInfoStr = QString(
        "UPDATE info_about_cars "
        "SET "
        "mileage = :mileage, "
        "insurance_end = :insuranceEnd, "
        "diagnostic_card_end = :diagnosticCardEnd, "
        "id_tire = :tireId "
        "WHERE id = :recordId"
        );

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto saveCarInfoQuery = wraper->query();
    saveCarInfoQuery.prepare(saveCarInfoStr);
    saveCarInfoQuery.bindValue(":recordId", _recordId);
    saveCarInfoQuery.bindValue(":mileage", mileage);
    saveCarInfoQuery.bindValue(":insuranceEnd", insuranceEnd);
    saveCarInfoQuery.bindValue(":diagnosticCardEnd", diagnosticCardEnd);
    saveCarInfoQuery.bindValue(":tireId", tireId);

    const auto saveCarInfoResult = saveCarInfoQuery.exec();
    if (!saveCarInfoResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastError().text();
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastQuery();
        return false;
    }

    return true;
}
