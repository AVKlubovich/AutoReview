#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequestManager;
    typedef QSharedPointer<WebRequestManager> WebRequestManagerShp;
}

namespace auto_review
{

    class Login :
            public core::Command,
            public core::CommandCreator<Login>
    {
        friend class QSharedPointer<Login>;

    private:
        Login(const Context& newContext);

    public:
        network::ResponseShp exec() override;

    private:
        void setError(const QString & err, const quint64 status = -1);

    private:
        network::WebRequestManagerShp _webManager;

        const QString ERROR_LOGIN_OR_PASSWORD = "Неверный логин и/или пароль";
    };

}
