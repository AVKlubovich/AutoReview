#include "Common.h"
#include "ServerRequestHandler.h"

#include "server-core/Commands/CommandFactory.h"

#include "network-core/RequestsManager/ResponseServerError.h"


using namespace auto_review;

ServerRequestHandler::ServerRequestHandler(const core::Command::Context& context)
    : RequestHandler(context)
{
    const network::Packet packet = _context._packet;
    const QSharedPointer<core::Responce> &responce = _context._responce;

    responce->setHeaders(packet.headers());

    _commandType = packet.body().toMap()["type_command"].toString();
    if (_commandType.isEmpty())
    {
//        support new structured HEAD/BODY packets
        _commandType = packet.body().toMap()["head"].toMap()["type"].toString();
    }
    qDebug() << "Creating server command" << _commandType;
    _command = core::CommandFactory::instance().create(_commandType, _context);
}

QSharedPointer<network::Response> ServerRequestHandler::process()
{
//    core::Command::Context newContext;
    const network::Packet packet = _context._packet;
    const QSharedPointer<core::Responce> &responce = _context._responce;

//    responce->setHeaders(packet.headers());

//    QString commandType = packet.body().toMap()["type_command"].toString();
//    if (commandType.isEmpty())
//    {
//        //support new structured HEAD/BODY packets
//        commandType = packet.body().toMap()["head"].toMap()["type"].toString();
//    }
//    qDebug() << "Creating server command" << commandType;
//    _command = core::CommandFactory::instance().create(commandType, _context);

    QSharedPointer<network::Response> response;
    if (!_command.isNull())
    {
        try
        {
            response = _command->exec();
        }
        catch (const std::runtime_error & ex)
        {
            QString errMsg = QString("Error executing server command '%1': %2").arg(_commandType).arg(ex.what());
            response.reset(new network::ResponseServerError(errMsg, QSharedPointer<network::Request>()));
        }
    }
    else
    {
        QString errMessage = QString("Unrecognized command request: '%1' (found in 'head/type' or 'type_command' request element)" ).arg(_commandType);
        qCritical() << errMessage;
        response.reset(new network::ResponseServerError(errMessage, QSharedPointer<network::Request>()));//TODO: pass request instead of NULL after create request factory
    }

    responce->setHeaders(packet.headers());
    return response;
}

void ServerRequestHandler::processSlow()
{
    auto callback = [this]()
    {
        this->_callback();
    };

    _command->execSlow(callback);
}

network::ResponseShp ServerRequestHandler::getSlowResponse()
{
    const network::Packet packet = _context._packet;
    const QSharedPointer<core::Responce> &responce = _context._responce;
    QSharedPointer<network::Response> response;
    if (!_command.isNull())
    {
        try
        {
            response = _command->getSlowResponse();
        }
        catch (const std::runtime_error & ex)
        {
            QString errMsg = QString("Error executing server command '%1': %2").arg(_commandType).arg(ex.what());
            response.reset(new network::ResponseServerError(errMsg, QSharedPointer<network::Request>()));
        }
    }
    else
    {
        QString errMessage = QString("Unrecognized command request: '%1' (found in 'head/type' or 'type_command' request element)" ).arg(_commandType);
        qCritical() << errMessage;
        response.reset(new network::ResponseServerError(errMessage, QSharedPointer<network::Request>()));//TODO: pass request instead of NULL after create request factory
    }

    responce->setHeaders(packet.headers());
    return response;
}


QSharedPointer<core::RequestHandler> ServerRequestHandlerFactory::create(const core::Command::Context& context)
{
    return QSharedPointer<ServerRequestHandler>::create(context);
}
