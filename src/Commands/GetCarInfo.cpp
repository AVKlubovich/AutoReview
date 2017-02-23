#include "Common.h"
#include "GetCarInfo.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

RegisterCommand(auto_review::GetCarInfo, "get_car_info")


using namespace auto_review;

GetCarInfo::GetCarInfo(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetCarInfo::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("login") ||
        !bodyData.contains("password") ||
        !bodyData.contains("id_car"))
    {
        sendError("Do not send field", "field_error", signature());
        return network::ResponseShp();
    }

    const auto carId = bodyData["id_car"].toInt();

    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    QVariantMap userData;
    userData["type_query"] = "get_autos_data";
    userData["auto_id"] = QString::number(carId);
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
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: field not sended";
        return network::ResponseShp();
    }

    const auto status = map["status"].toInt();
    if (status != 1)
    {
        const auto& errorList = map["error"].toList();
        const auto& errorStr = errorList.first().toString();
        sendError(errorStr, "remove_server_error", signature());
        return network::ResponseShp();
    }

    const auto& array = map["array"].toList();
    if (array.isEmpty())
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: array is empty";
        return network::ResponseShp();
    }

    auto infoMap = array.first().toMap();

    const auto& wraper = database::DBManager::instance().getDBWraper();
    wraper->startTransaction();

    if (!checkDriversPin(infoMap))
    {
        wraper->rollback();
        return network::ResponseShp();
    }

    const auto& selectDataStr = QString(
        "SELECT "
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
        "WHERE info_about_cars.id_car = :carId "
        "ORDER BY info_about_cars.id DESC LIMIT 1"
        );

    auto selectDataQuery = wraper->query();
    selectDataQuery.prepare(selectDataStr);
    selectDataQuery.bindValue(":carId", carId);

    const auto selectDataResult = selectDataQuery.exec();
    if (!selectDataResult)
    {
        wraper->rollback();
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << selectDataQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectDataQuery.lastQuery();
        return network::ResponseShp();
    }

    const auto& infoList = database::DBHelpers::queryToVariant(selectDataQuery);
    if (!infoList.isEmpty())
    {
        const auto& dbInfo = infoList.last().toMap();
        for (auto it = dbInfo.begin(); it != dbInfo.end(); ++it)
            infoMap[it.key()] = it.value();
    }

    wraper->commit();

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["info"] = QVariant::fromValue(infoMap);

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}

bool GetCarInfo::checkDriversPin(const QVariantMap& data)
{
    const auto& wraper = database::DBManager::instance().getDBWraper();

    const QString& selectPinStr = QString(
        "SELECT * "
        "FROM drivers "
        "WHERE id = :driverId"
        );

    const quint64 driverId = data["id_driver"].toULongLong();
    auto selectPinQuery = wraper->query();
    selectPinQuery.prepare(selectPinStr);
    selectPinQuery.bindValue(":driverId", driverId);

    const bool selectPinResult = selectPinQuery.exec();
    if (!selectPinResult)
    {
        sendError("Can not select drivers pin", "db_error", signature());
        qDebug() << __FUNCTION__ << selectPinQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectPinQuery.lastQuery();
        return false;
    }

    const auto& resultList = database::DBHelpers::queryToVariantMap(selectPinQuery);
    if (!resultList.isEmpty())
    {
        return true;
    }

    const QString& insertPinStr = QString(
        "INSERT INTO drivers "
        "(id, pin) "
        "VALUES "
        "(:driverId, :pin)"
        );

    auto insertPinQuery = wraper->query();
    insertPinQuery.prepare(insertPinStr);
    insertPinQuery.bindValue(":driverId", driverId);
    const quint64 newPin = qrand() % 10000;
    insertPinQuery.bindValue(":pin", newPin);

    const bool insertPinResult = insertPinQuery.exec();
    if (!insertPinResult)
    {
        sendError("Can not insert new pin", "db_error", signature());
        qDebug() << __FUNCTION__ << insertPinQuery.lastError().text();
        qDebug() << __FUNCTION__ << insertPinQuery.lastQuery();
        return false;
    }

    if (!informDriver(data, newPin))
    {
        sendError("Can not inform driver about his new pin", "remote_server_error", signature());
        return false;
    }

    return true;
}

bool GetCarInfo::informDriver(const QVariantMap& infoData, const quint64 driverPin)
{
    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    const QString& smsText = QString(
        "Ваш новый pin для сдачи и получения машины: %1")
        .arg(driverPin);
//    const QString& driverPhone = infoData["phone_number"].toString();
    const QString& driverPhone = QString("+375295836603");

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();
    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    QVariantMap userData;
    userData["type_query"] = "send_sms_api";
    userData["sms_text"] = smsText;
    userData["sms_phone"] = driverPhone;
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
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: Bad response from remote server";
        return false;
    }

    const auto status = map["status"].toInt();
    if (status != 1)
    {
        const auto& errorList = map["error"].toList();
        const auto& errorStr = errorList.first().toString();
        sendError(errorStr, "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error:" << errorList;
        return false;
    }

    return true;
}
