#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    class GetCarDamage :
            public core::Command,
            public core::CommandCreator<GetCarDamage>
    {
        friend class QSharedPointer<GetCarDamage>;

    private:
        GetCarDamage(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;

        const QString ERROR_LOGIN_OR_PASSWORD = "Неверный логин и/или пароль";
    };

}
