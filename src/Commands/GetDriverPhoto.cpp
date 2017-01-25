#include "Common.h"
#include "GetDriverPhoto.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

#include "utils/Settings/SettingsFactory.h"
#include "utils/Settings/Settings.h"

RegisterCommand(auto_review::GetDriverPhoto, "get_driver_photo")


using namespace auto_review;

GetDriverPhoto::GetDriverPhoto(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetDriverPhoto::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());

    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();

    if (!bodyData.contains("login") ||
        !bodyData.contains("password") ||
        !bodyData.contains("id_driver"))
    {
        sendError("Do not send field", "field_error", signature());
        return network::ResponseShp();
    }

    const auto driverId = bodyData["id_driver"].toInt();

    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    auto settings = utils::SettingsFactory::instance().settings("server-core");
    settings.beginGroup("ApiGeneral");
    QStringList urls;
    urls << settings["UrlImage_1"].toString();
    urls << settings["UrlImage_2"].toString();
    urls << settings["UrlImage_3"].toString();
    const auto& urlImg = urls.at(qrand() % urls.count());

    webManager->setSingleUrl(urlImg);

//    http://192.168.211-213.30:81/api/api_images_taxi_spb.php?
//    name=$NAME&
//    pass=$PASS&
//    type_query=get_image_driver&
//    user_login=$USER_LOGIN&
//    user_pass=$USER_PASS_MD5&
//    id=1&
//    hash=MD5($ID + drivera)&
//    CITY=$CITY&
//    echo_image=1

    QVariantMap userData;
    userData["type_query"] = "get_image_driver";
    userData["id"] = QString::number(driverId);
    userData["CITY"] = "1";
    userData["echo_image"] = "1";
    userData["user_login"] = userLogin;
    userData["user_pass"] = QString(QCryptographicHash::hash(userPass.toStdString().data(), QCryptographicHash::Md5).toHex());
    const auto& driverData = QString("%1%2").arg(QString::number(driverId)).arg("drivera");
    userData["hash"] = QString(QCryptographicHash::hash(driverData.toStdString().data(),QCryptographicHash::Md5).toHex());
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
        "(SELECT maintenance.mileage FROM maintenance WHERE id_car = info_about_cars.id_car AND maintenance.is_big = 0 ORDER BY id DESC LIMIT 1) as last_maintenance, "
        "(SELECT maintenance.mileage FROM maintenance WHERE id_car = info_about_cars.id_car AND maintenance.is_big = 1 ORDER BY id DESC LIMIT 1) as last_big_maintenance, "
        "info_about_cars.date_create AS last_timestamp "
        "FROM info_about_cars "
        "INNER JOIN maintenance "
        "ON (maintenance.id_car = info_about_cars.id_car) "
        "WHERE info_about_cars.id_car = :carId "
        "ORDER BY info_about_cars.id DESC LIMIT 1"
        );

    auto selectDataQuery = wraper->query();
    selectDataQuery.prepare(selectDataStr);
    selectDataQuery.bindValue(":carId", driverId);

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
