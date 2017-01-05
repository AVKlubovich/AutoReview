#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class GetStartingData
        : public core::Command
        , public core::CommandCreator<GetStartingData>
    {
        friend class QSharedPointer<GetStartingData>;

        GetStartingData(const Context& newContext);
    public:
        ~GetStartingData() override = default;
        QSharedPointer<network::Response> exec() override;
    private:
        QVariantList listOfPossibleDamages(const QList<QVariant> &list);
        QVariantList listOfElementsCoordinates(const QList<QVariant> &list);
    };

}
