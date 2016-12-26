#pragma once

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"


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
        QSharedPointer<network::Response> exec() override;
    };

}
