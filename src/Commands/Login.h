#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class Login :
            public core::Command,
            public core::CommandCreator<Login>
    {
        friend class QSharedPointer<Login>;

        Login(const Context& newContext);
    public:
        ~Login() override = default;

        QSharedPointer<network::Response> exec() override;
    };

}
