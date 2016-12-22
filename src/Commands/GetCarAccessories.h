#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    class GetCarAccessories :
            public core::Command,
            public core::CommandCreator<GetCarAccessories>
    {
        friend class QSharedPointer<GetCarAccessories>;

    private:
        GetCarAccessories(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        QSharedPointer<network::WebRequestManager> _webManager;
    };

}
