#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class AttachDriverToCar :
            public core::Command,
            public core::CommandCreator<AttachDriverToCar>
    {
        friend class QSharedPointer<AttachDriverToCar>;

    private:
        AttachDriverToCar(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
