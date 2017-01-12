#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class RemoveDriverFromCar :
            public core::Command,
            public core::CommandCreator<RemoveDriverFromCar>
    {
        friend class QSharedPointer<RemoveDriverFromCar>;

    private:
        RemoveDriverFromCar(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
