#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    // NOTE: пересменка
    class GetRelisedCarNumber :
            public core::Command,
            public core::CommandCreator<GetRelisedCarNumber>
    {
        friend class QSharedPointer<GetRelisedCarNumber>;

    private:
        GetRelisedCarNumber(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;
    };

}
