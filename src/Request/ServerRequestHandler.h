#pragma once

#include "server-core/Request/RequestHandler.h"


namespace auto_review
{

    class ServerRequestHandler : public core::RequestHandler
    {
    public:
        ServerRequestHandler(const core::Command::Context& context);
        virtual ~ServerRequestHandler() = default;

        QSharedPointer<network::Response> process() override;

        void processSlow() override;
        network::ResponseShp getSlowResponse() override;

    private:
        QString _commandType;
    };


    class ServerRequestHandlerFactory
        : public core::RequestHandlerFactory
    {
    public:
        ServerRequestHandlerFactory() = default;
        virtual ~ServerRequestHandlerFactory() = default;

        QSharedPointer<core::RequestHandler> create(const core::Command::Context& context) override;
    };

}
