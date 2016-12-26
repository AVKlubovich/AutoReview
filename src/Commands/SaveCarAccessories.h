#pragma once

#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"


namespace auto_review
{

    class SaveCarAccessories :
            public core::Command,
            public core::CommandCreator<SaveCarAccessories>
    {
        friend class QSharedPointer<SaveCarAccessories>;

    private:
        SaveCarAccessories(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;
    };

}
