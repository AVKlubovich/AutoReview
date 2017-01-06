#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class GetCarInfo :
            public core::Command,
            public core::CommandCreator<GetCarInfo>
    {
        friend class QSharedPointer<GetCarInfo>;

    private:
        GetCarInfo(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
