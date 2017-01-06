#include "Common.h"
#include "GetStartingData.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

#include "Definitions.h"

RegisterCommand(auto_review::GetStartingData, "get_server_data")


using namespace auto_review;

GetStartingData::GetStartingData(const Context& newContext)
    : Command(newContext)
{
}

network::ResponseShp GetStartingData::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto& responce = _context._responce;
    responce->setHeaders(_context._packet.headers());

    QMap<QString, QString> mapTableComplaints;
    mapTableComplaints["accessories"]   = "SELECT * FROM accessories";
    mapTableComplaints["damage"]        = "SELECT * FROM type_damage_cars";
    mapTableComplaints["tires"]         = "SELECT * FROM tires_type";
    mapTableComplaints["check_type"]    = "SELECT * FROM check_type";
    mapTableComplaints["car_elements"]  = "SELECT "
                                          "config_elements_of_cars.id, config_elements_of_cars.element, config_elements_of_cars.color, damage_from_elements.id_damage "
                                          "FROM config_elements_of_cars "
                                          "LEFT JOIN damage_from_elements "
                                          "ON (config_elements_of_cars.id = damage_from_elements.id_element)";
    mapTableComplaints["car_positions"] = "SELECT "
                                          "pos.*, car_pos.id_car_element, car_pos.\"axis_X\", car_pos.\"axis_Y\" "
                                          "FROM config_car_positions AS pos "
                                          "INNER JOIN config_elements_for_car_positions AS car_pos "
                                          "ON (pos.id = car_pos.id_car_position)";

    const auto wraper = database::DBManager::instance().getDBWraper();
    auto selectQuery = wraper->query();

    QVariantMap resultMap;
    QMapIterator<QString, QString> iteratorMapTableComplaints(mapTableComplaints);
    while (iteratorMapTableComplaints.hasNext())
    {
        iteratorMapTableComplaints.next();

        selectQuery.prepare(iteratorMapTableComplaints.value());
        const auto selectComplaintsResult = wraper->execQuery(selectQuery);
        if (!selectComplaintsResult)
        {
            sendError(selectQuery.lastError().text(), "error", signature());
            qDebug() << selectQuery.lastError().text();
            return network::ResponseShp();
        }

        const auto& resultList = database::DBHelpers::queryToVariant(selectQuery);

        if (iteratorMapTableComplaints.key() == "car_elements")
        {
            resultMap[iteratorMapTableComplaints.key()] = listOfPossibleDamages(resultList);
            continue;
        }

        if (iteratorMapTableComplaints.key() == "car_positions")
        {
            resultMap[iteratorMapTableComplaints.key()] = listOfElementsCoordinates(resultList);
            continue;
        }

        resultMap[iteratorMapTableComplaints.key()] = QVariant::fromValue(resultList);
    }

    resultMap["type_command"] = signature();
    resultMap["status"] = 1;

    QVariantMap head;
    head["type"] = signature();

    QVariantMap result;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(resultMap);

    responce->setBody(QVariant::fromValue(result));

    return network::ResponseShp();
}

QVariantList GetStartingData::listOfPossibleDamages(const QList<QVariant> &list)
{
    QVariantMap mapObjectResult;

    for (const auto& item : list)
    {
        const auto& mapItem = item.toMap();
        const auto elementId = mapItem.value("id").toString();
        const auto element = mapItem.value("element");
        const auto id_damage = mapItem.value("id_damage").toInt();
        const auto& color = mapItem.value("color").toString();
        const QString damage = "damage";

        if (!mapObjectResult.contains(elementId))
        {
            QVariantMap mapObject;
            mapObject["id"] = elementId;
            mapObject["element"] = element;
            mapObject["color"] = color;
            mapObject[damage] = QVariant();

            mapObjectResult[elementId] = mapObject;
        }

        auto mapObject = mapObjectResult[elementId].toMap();

        auto list = mapObject[damage].toList();
        if (id_damage)
            list.append(id_damage);

        mapObject[damage] = list;
        mapObjectResult[elementId] = mapObject;
    }

    QVariantList listResult;
    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}

QVariantList GetStartingData::listOfElementsCoordinates(const QList<QVariant> &list)
{
    QVariantMap mapObjectResult;

    for (const auto &item : list)
    {
        const auto& mapItem = item.toMap();
        const auto& urlId = mapItem.value("id").toString();
        const auto& foregroundUrl = mapItem.value("foreground_url").toString();
        const auto& backgroundUrl = mapItem.value("background_url").toString();
        const auto idCarElement = mapItem.value("id_car_element");
        const auto& axisX = mapItem.value("axis_X");
        const auto& axisY = mapItem.value("axis_Y");
        const auto& coordinates = "car_details";

        if (!mapObjectResult.contains(urlId))
        {
            QVariantMap mapObject;
            mapObject["id"] = urlId;
            mapObject["foreground_url"] = checkIpAddress(foregroundUrl);
            mapObject["background_url"] = checkIpAddress(backgroundUrl);
            mapObject[coordinates] = QVariant();

            mapObjectResult[urlId] = mapObject;
        }

        auto mapObject = mapObjectResult[urlId].toMap();
        auto listCoordinates = mapObject[coordinates].toList();

        QVariantMap mapCoordinates;
        mapCoordinates["id_car_element"] = idCarElement;
        mapCoordinates["axis_X"] = axisX;
        mapCoordinates["axis_Y"] = axisY;
        listCoordinates.append(mapCoordinates);

        mapObject[coordinates] = listCoordinates;
        mapObjectResult[urlId] = mapObject;
    }

    QVariantList listResult;
    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}

const QString &GetStartingData::checkIpAddress(QString url)
{
    const auto& remoteAddr = QString(_context._packet.headers().header("REMOTE_ADDR"));
    if (remoteAddr.contains(OUR_MASK))
    {
        const QString& newUrl = url.replace(VM_IP, INSIDE_IP);
        return newUrl;
    }
    else
    {
        const QString& newUrl = url.replace(VM_IP, OUTSIDE_IP);
        return newUrl;
    }
}
