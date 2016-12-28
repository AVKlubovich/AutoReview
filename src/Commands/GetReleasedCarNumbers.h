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
    class GetReleasedCarNumber :
            public core::Command,
            public core::CommandCreator<GetReleasedCarNumber>
    {
        friend class QSharedPointer<GetReleasedCarNumber>;

    private:
        GetReleasedCarNumber(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;
    };

}
