#include "Common.h"
#include "WSGetStartData.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "Definitions.h"

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
        "info_about_cars.id_car, "
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
        "WHERE info_about_cars.id IN (SELECT max(id) FROM info_about_cars GROUP BY id_car) "
//        "AND info_about_cars.id_car IN (1, 4) "
//        "AND info_about_cars.status = :status"
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

    if (carsList.isEmpty())
    {
        sendEmptyResponse();
        return network::ResponseShp();
    }

    QStringList carIdsList;
    for (const auto& carInfo : carsList)
    {
        const QString& carId = carInfo.toMap()["id_car"].toString();
        carIdsList << carId;
    }

    bool okD = false;
    const auto& damages = getDamages(carIdsList.join(","), okD);
    if (!okD)
    {
        return network::ResponseShp();
    }

    bool okR = false;
    const auto& carsRemoteList = getRemoteData(carIdsList.join(","), okR);
    if (!okR)
    {
        return network::ResponseShp();
    }

    QMap<quint64, QVariantMap> carsRemoteInfoMap;
    for (auto& carInfo : carsRemoteList)
    {
        const QVariantMap& carInfoMap = carInfo.toMap();
        const quint64 carId = carInfoMap["id"].toULongLong();
        carsRemoteInfoMap.insert(carId, carInfoMap);
    }

    QMap<quint64, QVariantMap> carsInfoMap;
    for (auto& carInfo : carsList)
    {
        const QVariantMap& carInfoMap = carInfo.toMap();
        const quint64 carId = carInfoMap["id_car"].toULongLong();
        carsInfoMap.insert(carId, carInfoMap);
    }

    for (const auto& damage : damages)
    {
        const QVariantMap& damageMap = damage.toMap();
        const quint64 carId = damageMap["id_car"].toULongLong();
        QVariantMap& carInfoMap = carsInfoMap[carId];
        QVariantList damageList = carInfoMap["damages"].toList();
        damageList << damageMap;
        carInfoMap["damages"] = damageList;
    }

    for (const auto& carRemoteInfoMap : carsRemoteInfoMap.values())
    {
        const quint64 carId = carRemoteInfoMap["id"].toULongLong();
        QVariantMap& carInfoMap = carsInfoMap[carId];
        carInfoMap.unite(carRemoteInfoMap);
    }

    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    QVariantList carsListV;
    for (const auto& car : carsInfoMap.values())
        carsListV << QVariant::fromValue(car);
    body["cars"] = QVariant::fromValue(carsListV);

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}

QVariantList WSGetStartData::getRemoteData(const QString& carIdsList, bool& ok)
{
    const auto& incomingData = _context._packet.body().toMap();
    const auto& bodyData = incomingData.value("body").toMap();
    const auto& userLogin = bodyData["login"].toString();
    const auto& userPass = bodyData["password"].toString();

    auto webManager = network::WebRequestManager::instance();
    auto webRequest = network::WebRequestShp::create("type_query");

    QVariantMap userData;
    userData["type_query"] = "get_autos_data";
    userData["auto_id"] = carIdsList;
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

        ok = false;
        return QVariantList();
    }

    const auto status = map["status"].toInt();
    if (status != 1)
    {
        const auto& errorList = map["error"].toList();
        const auto& errorStr = errorList.first().toString();
        sendError(errorStr, "remove_server_error", signature());

        ok = false;
        return QVariantList();
    }

    const auto& carsRemoteList = map["array"].toList();
    if (carsRemoteList.isEmpty())
    {
        sendError("Bad response from remote server", "remove_server_error", signature());
        qDebug() << __FUNCTION__ << "error: array is empty";

        ok = false;
        return QVariantList();
    }

    ok = true;
    return carsRemoteList;
}

QVariantList WSGetStartData::getDamages(const QString &carIdsList, bool &ok)
{
    const auto& selectDamagesStr = QString(
        "SELECT "
        "id, "
        "id_car, "
        "comment, "
        "date_create, "
        "status, "
        "(SELECT element FROM config_elements_of_cars WHERE config_elements_of_cars.id = id_element_damage) AS element_name,"
        "(SELECT type FROM type_damage_cars WHERE type_damage_cars.id = type_damage) "
        "FROM car_damage "
        "WHERE id_car IN (%1)"
        ).arg(carIdsList);

    const auto& wraper = database::DBManager::instance().getDBWraper();
    auto selectDamagesQuery = wraper->query();
    selectDamagesQuery.prepare(selectDamagesStr);

    const auto selectDamagesResult = selectDamagesQuery.exec();
    if (!selectDamagesResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << selectDamagesQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectDamagesQuery.lastQuery();
        ok = false;
        return QVariantList();
    }

    const QVariantList& damages = database::DBHelpers::queryToVariant(selectDamagesQuery);
    QMap<QString, QVariantMap> damagesMap;
    for (const auto& damage : damages)
    {
        const QVariantMap& damageMap = damage.toMap();
        const QString& damageId = damageMap["id"].toString();
        damagesMap.insert(damageId, damageMap);
    }

    bool okPhotos = false;
    const auto& photos = getPhotos(QStringList(damagesMap.keys()).join(","), okPhotos);
    if (!okPhotos)
    {
        ok = false;
        return QVariantList();
    }

    for (const auto& photo : photos)
    {
        const QString& damageId = photo.toMap()["id_car_damage"].toString();
        QVariantMap& damageMap = damagesMap[damageId];
        QVariantList photosFromDamage = damageMap["photos"].toList();
        photosFromDamage << photo;
        damageMap["photos"] = photosFromDamage;
    }

    QVariantList damagesList;
    for (const auto& damage : damagesMap)
    {
        damagesList << damage;
    }

    ok = true;
    return damagesList;
}

QVariantList WSGetStartData::getPhotos(const QString &damageIdsList, bool &ok)
{
    const auto& selectPhotosStr = QString(
        "SELECT * FROM photos "
        "WHERE id_car_damage IN (%1)"
        ).arg(damageIdsList);

    const auto& wraper = database::DBManager::instance().getDBWraper();
    auto selectPhotosQuery = wraper->query();
    selectPhotosQuery.prepare(selectPhotosStr);

    const auto selectPhotosResult = selectPhotosQuery.exec();
    if (!selectPhotosResult)
    {
        sendError("Database error", "db_error", signature());
        qDebug() << __FUNCTION__ << selectPhotosQuery.lastError().text();
        qDebug() << __FUNCTION__ << selectPhotosQuery.lastQuery();
        ok = false;
        return QVariantList();
    }

    const QVariantList& photos = database::DBHelpers::queryToVariant(selectPhotosQuery);
    const QVariantList& newPhotos = checkIpAddress(photos);

    ok = true;
    return newPhotos;
}

QVariantList WSGetStartData::checkIpAddress(const QVariantList &photosList)
{
    const auto& remoteAddr = QString(_context._packet.headers().header("REMOTE_ADDR"));

    QVariantList newPhotosList;
    for (const auto& photo : photosList)
    {
        QString newUrl;
        if (remoteAddr.contains(OUR_MASK))
            newUrl = photo.toMap()["url"].toString().replace(VM_IP, INSIDE_IP);
        else
            newUrl = photo.toMap()["url"].toString().replace(VM_IP, OUTSIDE_IP);

        QVariantMap newPhoto = photo.toMap();
        newPhoto["url"] = newUrl;
        newPhotosList << QVariant::fromValue(newPhoto);
    }

    return newPhotosList;
}

void WSGetStartData::sendEmptyResponse()
{
    QVariantMap head;
    head["type"] = signature();

    QVariantMap body;
    body["status"] = 1;
    body["cars"] = QVariant::fromValue(QVariantList());

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));
}
