#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class GetReleasedCarNumber :
            public core::Command,
            public core::CommandCreator<GetReleasedCarNumber>
    {
        friend class QSharedPointer<GetReleasedCarNumber>;

    private:
        GetReleasedCarNumber(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
