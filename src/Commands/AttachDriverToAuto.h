#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    // NOTE: пересменка
    class AttachDriverToAuto :
            public core::Command,
            public core::CommandCreator<AttachDriverToAuto>
    {
        friend class QSharedPointer<AttachDriverToAuto>;

    private:
        AttachDriverToAuto(const Context& newContext);

    public:
        network::ResponseShp exec() override;
    };

}
