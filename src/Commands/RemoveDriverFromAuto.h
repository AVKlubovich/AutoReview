#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class RemoveDriverFromAuto :
            public core::Command,
            public core::CommandCreator<RemoveDriverFromAuto>
    {
        friend class QSharedPointer<RemoveDriverFromAuto>;

    private:
        RemoveDriverFromAuto(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
