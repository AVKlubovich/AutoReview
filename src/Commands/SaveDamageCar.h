#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class SaveDamageCar :
            public core::Command,
            public core::CommandCreator<SaveDamageCar>
    {
        friend class QSharedPointer<SaveDamageCar>;

    private:
        SaveDamageCar(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
