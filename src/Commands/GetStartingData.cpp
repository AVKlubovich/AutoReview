#include "Common.h"
#include "GetStartingData.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

#include "network-core/Packet/JsonConverter.h"

RegisterCommand(auto_review::GetStartingData, "get_server_data")


using namespace auto_review;

GetStartingData::GetStartingData(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetStartingData::exec()
{
    qDebug() << __FUNCTION__ << "was runned" << QDateTime::currentDateTime() << endl;

    auto & responce = _context._responce;
    responce->setHeaders(_context._packet.headers());

    QMap<QString, QString> mapTableComplaints;

    // complaints_schema
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
            return QSharedPointer<network::Response>();
        }

        const auto resultList = database::DBHelpers::queryToVariant(selectQuery);

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

    return QSharedPointer<network::Response>();
}

QVariantList GetStartingData::listOfPossibleDamages(const QList<QVariant> &list)
{
    QVariantMap mapObjectResult;
    QVariantList listResult;

    for (const auto &item : list)
    {
        auto mapItem = item.toMap();
        const auto id_element = mapItem.value("id").toString();
        const auto element = mapItem.value("element");
        const auto id_damage = mapItem.value("id_damage").toInt();
        const auto& color = mapItem.value("color").toString();
        const QString damage = "damage";

        if (!mapObjectResult.contains(id_element))
        {
            QVariantMap mapObject;
            mapObject["id"] = id_element;
            mapObject["element"] = element;
            mapObject["color"] = color;
            mapObject[damage] = QVariant();

            mapObjectResult[id_element] = mapObject;
        }

        auto mapObject = mapObjectResult[id_element].toMap();

        auto list = mapObject[damage].toList();
        if (id_damage)
            list.append(id_damage);

        mapObject[damage] = list;
        mapObjectResult[id_element] = mapObject;
    }

    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}

QVariantList GetStartingData::listOfElementsCoordinates(const QList<QVariant> &list)
{
    QVariantMap mapObjectResult;
    QVariantList listResult;

    for (const auto &item : list)
    {
        const auto& mapItem = item.toMap();
        const auto& id = mapItem.value("id").toString();
        const auto& foreground_url = mapItem.value("foreground_url");
        const auto& background_url = mapItem.value("background_url");
        const auto id_car_element = mapItem.value("id_car_element");
        const auto& axis_X = mapItem.value("axis_X");
        const auto& axis_Y = mapItem.value("axis_Y");
        const auto& coordinates = "car_details";

        if (!mapObjectResult.contains(id))
        {
            QVariantMap mapObject;
            mapObject["id"] = id;
            mapObject["foreground_url"] = foreground_url;
            mapObject["background_url"] = background_url;
            mapObject[coordinates] = QVariant();

            mapObjectResult[id] = mapObject;
        }

        auto mapObject = mapObjectResult[id].toMap();
        auto listCoordinates = mapObject[coordinates].toList();

        QVariantMap mapCoordinates;
        mapCoordinates["id_car_element"] = id_car_element;
        mapCoordinates["axis_X"] = axis_X;
        mapCoordinates["axis_Y"] = axis_Y;
        listCoordinates.append(mapCoordinates);

        mapObject[coordinates] = listCoordinates;
        mapObjectResult[id] = mapObject;
    }

    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}
