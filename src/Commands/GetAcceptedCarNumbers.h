#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    // NOTE: нет водителя и на ремзоне
    class GetAcceptedCarNumbers :
            public core::Command,
            public core::CommandCreator<GetAcceptedCarNumbers>
    {
        friend class QSharedPointer<GetAcceptedCarNumbers>;

    private:
        GetAcceptedCarNumbers(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;
    };

}
