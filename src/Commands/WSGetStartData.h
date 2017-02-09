#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class WSGetStartData :
            public core::Command,
            public core::CommandCreator<WSGetStartData>
    {
        friend class QSharedPointer<WSGetStartData>;

    private:
        WSGetStartData(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
