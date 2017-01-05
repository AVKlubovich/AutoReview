#include "Common.h"
#include "GetStartingData.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "database/DBManager.h"
#include "database/DBWraper.h"
#include "database/DBHelpers.h"

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
    mapTableComplaints["accessories"]  = "SELECT * FROM accessories";
    mapTableComplaints["damage"]       = "SELECT * FROM type_damage_cars";
    mapTableComplaints["tires"]        = "SELECT * FROM tires_type";
    mapTableComplaints["check_type"]   = "SELECT * FROM check_type";
    mapTableComplaints["car_elements"] = "SELECT "
                                         "elements_of_cars.id, elements_of_cars.element, damage_from_elements.id_damage "
                                         "FROM elements_of_cars "
                                         "LEFT JOIN damage_from_elements "
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
            return network::ResponseShp();
        }

        const auto& resultList = database::DBHelpers::queryToVariant(selectQuery);
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
        const auto damageId = mapItem.value("id_damage");
        const QString damage = "damage";

        if (!mapObjectResult.contains(elementId))
        {
            QVariantMap mapObject;
            mapObject["id"] = elementId;
            mapObject["element"] = element;
            mapObject[damage] = QVariant();

            mapObjectResult[elementId] = mapObject;
        }

        auto mapObject = mapObjectResult[elementId].toMap();

        auto list = mapObject[damage].toList();
        list.append(damageId);

        mapObject[damage] = list;
        mapObjectResult[elementId] = mapObject;
    }

    QVariantList listResult;
    for (auto element = mapObjectResult.begin(); element != mapObjectResult.end(); ++element)
        listResult.append(element.value());

    return listResult;
}
