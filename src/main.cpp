#include "Common.h"

#include "Core/Core.h"

#ifdef Q_OS_LINUX
# include <unistd.h>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef Q_OS_LINUX
    if(QCoreApplication::arguments().contains("--daemon"))
    {
        daemon(0,0);
    }
#endif

    auto& core = auto_review::Core::instance();
    if (!core.init())
    {
        core.done();
        return -1;
    }

    a.exec();

    core.done();

    return 0;
}
