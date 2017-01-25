#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class GetDriverPhoto :
            public core::Command,
            public core::CommandCreator<GetDriverPhoto>
    {
        friend class QSharedPointer<GetDriverPhoto>;

    private:
        GetDriverPhoto(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
