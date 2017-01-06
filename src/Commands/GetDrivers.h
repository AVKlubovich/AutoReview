#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class GetDrivers :
            public core::Command,
            public core::CommandCreator<GetDrivers>
    {
        friend class QSharedPointer<GetDrivers>;

    private:
        GetDrivers(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
