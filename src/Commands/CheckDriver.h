#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class CheckDriver :
            public core::Command,
            public core::CommandCreator<CheckDriver>
    {
        friend class QSharedPointer<CheckDriver>;

    private:
        CheckDriver(const Context& newContext);

    public:
        network::ResponseShp exec() override;

    private:
        bool createDefaultPin(const qint64 driverId);
    };

}
