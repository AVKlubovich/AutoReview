#pragma once


#include "server-core/Commands/CommandFactory.h"
#include "server-core/Responce/Responce.h"

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
        QVariantList listDamages(const QVariantList &list);
    };

}
