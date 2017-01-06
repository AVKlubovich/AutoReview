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
        QVariantList listOfElementsCoordinates(const QList<QVariant> &list);
        const QString addressIpSubstitution(QString str);

    private:
        const QString externalNetwork = "134.17.26.128:6662";
        const QString internalNetwork = "192.168.202.222:6662";
    };
}

