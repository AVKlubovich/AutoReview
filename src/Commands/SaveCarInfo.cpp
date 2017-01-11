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

    const auto carId = bodyData["id_car"].toLongLong();
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
        "WHERE id = (SELECT id from info_about_cars WHERE id_car = :carId ORDER BY id DESC LIMIT 1)"
        );

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto saveCarInfoQuery = wraper->query();
    saveCarInfoQuery.prepare(saveCarInfoStr);
    saveCarInfoQuery.bindValue(":mileage", mileage);
    saveCarInfoQuery.bindValue(":insuranceEnd", insuranceEnd);
    saveCarInfoQuery.bindValue(":diagnosticCardEnd", diagnosticCardEnd);
    saveCarInfoQuery.bindValue("tireId", tireId);
    saveCarInfoQuery.bindValue(":carId", carId);

    const auto saveCarInfoResult = saveCarInfoQuery.exec();
    if (!saveCarInfoResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastError().text();
        qDebug() << __FUNCTION__ << saveCarInfoQuery.lastQuery();
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
