#pragma once

#include "server-core/Commands/UserCommand.h"


namespace network
{
    class WebRequest;
    class WebRequestManager;
}

namespace auto_review
{

    class SaveDamageCar :
            public core::Command,
            public core::CommandCreator<SaveDamageCar>
    {
        friend class QSharedPointer<SaveDamageCar>;

    private:
        SaveDamageCar(const Context& newContext);

    public:
        QSharedPointer<network::Response> exec() override;

    private:
        void setError(const QString & err);

    private:
        QSharedPointer<network::WebRequestManager> _webManager;

        const QString ERROR_LOGIN_OR_PASSWORD = "Неверный логин и/или пароль";
    };

}
