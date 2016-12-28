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
    class GetCarInfo :
            public core::Command,
            public core::CommandCreator<GetCarInfo>
    {
        friend class QSharedPointer<GetCarInfo>;

    private:
        GetCarInfo(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;
    };

}
