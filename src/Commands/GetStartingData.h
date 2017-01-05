#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class GetStartingData
        : public core::Command
        , public core::CommandCreator<GetStartingData>
    {
        friend class QSharedPointer<GetStartingData>;

    private:
        GetStartingData(const Context& newContext);

    public:
        network::ResponseShp exec() override;

    private:
        QVariantList listOfPossibleDamages(const QList<QVariant> &list);
    };

}
