#pragma once

#include "server-core/Commands/UserCommand.h"


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
        network::ResponseShp exec() override;

    private:
        QVariantList listDamages(const QVariantList &list);
        QVariantList checkIpAddress(const QVariantList &list);
    };

}
