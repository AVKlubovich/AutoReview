#pragma once

#include "server-core/Commands/UserCommand.h"


namespace auto_review
{

    class SaveCarInfo :
            public core::Command,
            public core::CommandCreator<SaveCarInfo>
    {
        friend class QSharedPointer<SaveCarInfo>;

    private:
        SaveCarInfo(const Context& newContext);

    public:
        network::ResponseShp exec() override;

    private:
        bool insertNewData(const QVariantMap& bodyData);
        bool updateNewData(const QVariantMap& bodyData);

    private:
        quint64 _recordId = 0;
    };

}
