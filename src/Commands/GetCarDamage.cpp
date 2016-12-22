#include "Common.h"
#include "GetCarDamage.h"

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

#include "network-core/RequestsManager/Users/ResponseLogin.h"
#include "network-core/RequestsManager/Users/RequestLogin.h"

#include "web-exchange/WebRequestManager.h"
#include "web-exchange/WebRequest.h"

#include "database/DBHelpers.h"
#include "database/DBManager.h"
#include "database/DBWraper.h"

RegisterCommand(auto_review::GetCarDamage, "get_car_damage")


using namespace auto_review;

GetCarDamage::GetCarDamage(const Context& newContext)
    : Command(newContext)
{
}

QSharedPointer<network::Response> GetCarDamage::exec()
{
    auto& response = _context._responce;
    response->setHeaders(_context._packet.headers());
    auto incomingData = _context._packet.body().toMap();
    auto uData = incomingData.value("body").toMap();

    const auto id = uData["id"].toString();
    const auto wraper = database::DBManager::instance().getDBWraper();
    auto addQuery = wraper->query();

    const auto sqlQuery = QString("SELECT"
                                  " car_damage.id_car AS id,"
                                  " car_damage.comment,"
                                  " elements_of_cars.element,"
                                  " type_damage_cars.type"
                                  " FROM car_damage"
                                  " INNER JOIN elements_of_cars ON (elements_of_cars.id = car_damage.id_element_damage)"
                                  " INNER JOIN type_damage_cars ON (type_damage_cars.id = car_damage.type_damage)"
                                  " WHERE car_damage.id_car=:id");
    addQuery.prepare(sqlQuery);
    addQuery.bindValue(":id", id);
    bool addCarQueryResult = wraper->execQuery(addQuery);

    if (!addCarQueryResult)
        setError("error select car_damage");
    const auto listCar = database::DBHelpers::queryToVariant(addQuery);

    const auto sqlQueryPhotos = QString("SELECT *"
                                  " FROM photos"
                                  " WHERE photos.id_car=:id");
    addQuery.prepare(sqlQueryPhotos);
    addQuery.bindValue(":id", id);
    bool addPhotosQueryResult = wraper->execQuery(addQuery);

    if (!addPhotosQueryResult)
        setError("error select photos");
    const auto listPhotos = database::DBHelpers::queryToVariant(addQuery);

    QVariantMap body, head, result;
    head["type"] = signature();
    body["status"] = 1;
    body["car"] = listCar;
    body["photos"] = listPhotos;

    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));

    return QSharedPointer<network::Response>();
}

void GetCarDamage::setError(const QString& err)
{
    QVariantMap body;
    QVariantMap head;
    QVariantMap result;
    head["type"] = signature();
    body["status"] = -1;
    body["error"] = err;
    result["head"] = QVariant::fromValue(head);
    result["body"] = QVariant::fromValue(body);
    _context._responce->setBody(QVariant::fromValue(result));
}
