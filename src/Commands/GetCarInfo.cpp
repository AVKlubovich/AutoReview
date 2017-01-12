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

//    if (!bodyData.contains("login") ||
//        !bodyData.contains("password") ||
//        !bodyData.contains("id_car"))
//    {
//        sendError("Do not send field", "field_error", signature());
//        return network::ResponseShp();
//    }

    const auto carId = bodyData["id_car"].toInt();

//    const auto& userLogin = bodyData["login"].toString();
//    const auto& userPass = bodyData["password"].toString();

    const QString& userLogin = "test_andrej";
    const QString& userPass = "andrttest";

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

    const auto& selectDataStr = QString(
        "SELECT "
        "info_about_cars.mileage, "
        "insurance_end, "
        "diagnostic_card_end, "
        "id_tire, "
        "(SELECT maintenance.mileage FROM maintenance WHERE id_car = info_about_cars.id_car ORDER BY id DESC LIMIT 1) as last_maintenance "
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
