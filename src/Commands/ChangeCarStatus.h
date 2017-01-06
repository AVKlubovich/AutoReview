#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class ChangeCarStatus :
            public core::Command,
            public core::CommandCreator<ChangeCarStatus>
    {
        friend class QSharedPointer<ChangeCarStatus>;

    private:
        ChangeCarStatus(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
