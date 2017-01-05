#pragma once

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"


namespace auto_review
{

    class GetCarAccessories :
            public core::Command,
            public core::CommandCreator<GetCarAccessories>
    {
        friend class QSharedPointer<GetCarAccessories>;

    private:
        GetCarAccessories(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
