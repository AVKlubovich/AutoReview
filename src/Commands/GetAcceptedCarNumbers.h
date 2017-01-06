#pragma once

#include "server-core/Commands/UserCommand.h"


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
        network::ResponseShp exec() override;
    };

}
