#pragma once

#include "server-core/Core/Core.h"
#include "utils/BaseClasses/Singleton.h"


namespace utils
{
    class Logger;
    typedef QSharedPointer<Logger> LoggerShp;
}

namespace core
{
    class Server;
    typedef QSharedPointer<Server> ServerShp;

    class RequestHandlerFactory;
    typedef QSharedPointer<RequestHandlerFactory> RequestHandlerFactoryShp;
}

namespace auto_review
{

    class Core : public utils::Singleton<Core>
    {
    public:
        Core();
        ~Core() = default;

        bool init();
        void done();

    private:
        void readConfig();
        bool initLoger();
        bool initServer();
        bool initDBManager();
        bool initCommandFactory();
        bool initWebManager();

    private:
        utils::LoggerShp _logger;
        core::ServerShp _server;
    };

}
