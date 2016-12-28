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
    mapTableComplaints["accessories"]  = "SELECT * FROM accessories";
    mapTableComplaints["damage"]       = "SELECT * FROM type_damage_cars";
    mapTableComplaints["tires"]        = "SELECT * FROM tires_type";
    mapTableComplaints["check_type"]   = "SELECT * FROM check_type";
    mapTableComplaints["car_elements"] = "SELECT "
                                         "elements_of_cars.id, elements_of_cars.element, damage_from_elements.id_damage "
                                         "FROM elements_of_cars "
                                         "INNER JOIN damage_from_elements "
                                         "ON (elements_of_cars.id = damage_from_elements.id_element)";


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
        if (iteratorMapTableComplaints.key() != "car_elements")
            resultMap[iteratorMapTableComplaints.key()] = QVariant::fromValue(resultList);
        else
            resultMap[iteratorMapTableComplaints.key()] = listOfPossibleDamages(resultList);
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

    for (auto item : list)
    {
        auto mapItem = item.toMap();
        const auto id_element = mapItem.value("id").toString();
        const auto element = mapItem.value("element");
        const auto id_damage = mapItem.value("id_damage");
        const QString damage = "damage";

        if (!mapObjectResult.contains(id_element))
        {
            QVariantMap mapObject;
            mapObject["id"] = id_element;
            mapObject["element"] = element;
            mapObject[damage] = QVariant();

            mapObjectResult[id_element] = mapObject;
        }

        auto mapObject = mapObjectResult[id_element].toMap();

        auto list = mapObject[damage].toList();
        list.append(id_damage);

        mapObject[damage] = list;
        mapObjectResult[id_element] = mapObject;
    }

    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}
