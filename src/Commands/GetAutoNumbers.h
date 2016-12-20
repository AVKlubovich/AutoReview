#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    class GetAutoNumbers :
            public core::Command,
            public core::CommandCreator<GetAutoNumbers>
    {
        friend class QSharedPointer<GetAutoNumbers>;

    private:
        GetAutoNumbers(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;
    };

}
