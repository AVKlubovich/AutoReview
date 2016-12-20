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
    class GetReceptionCarNumbers :
            public core::Command,
            public core::CommandCreator<GetReceptionCarNumbers>
    {
        friend class QSharedPointer<GetReceptionCarNumbers>;

    private:
        GetReceptionCarNumbers(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;
    };

}
