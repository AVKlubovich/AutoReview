#pragma once

#include "server-core/Commands/UserCommand.h"


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
        network::ResponseShp exec() override;
    };

}
