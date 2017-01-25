#include "Common.h"
#include "Core.h"

#include "server-core/Server/Server.h"
#include "server-core/Commands/CommandFactory.h"

#include "Request/ServerRequestHandler.h"

#include "utils/Settings/SettingsFactory.h"
#include "utils/Settings/Settings.h"

#include "utils/Logging/Logger.h"
#include "utils/Logging/Devices/FileDevice.h"
#include "utils/Logging/Devices/DebuggerDevice.h"
#include "utils/Logging/Devices/ConsoleDevice.h"

#include "database/DBManager.h"

#include "web-exchange/WebRequestManager.h"


using namespace auto_review;

Core::Core()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    utils::Settings::Options config = { "configuration/server.ini", true };
    utils::SettingsFactory::instance().registerSettings("server-core", config);

    auto settings = utils::SettingsFactory::instance().settings("server-core");
    settings =
    {
        // FastCGI
        { "FastCGI/ConnectionString", "localhost:26000" },
        { "FastCGI/Backlog", 1 },      // Number of requests in queue
        { "FastCGI/MaxJobs", 1 },      // Maximum number of jobs
        { "FastCGI/LogTraffic", true },// Logging of HTTP traffic enabled

        // Multi-threading (0 - auto detect)
        { "MultiThreading/FastThreadCound", 0 },
        { "MultiThreading/LongThreadCound", 0 },

        // Logs
        { "Log/FlushInterval", 1000 },
        { "Log/PrefixName", "auto_review.log" },
        { "Log/Dir", "./logs/" },
        { "Log/MaxSize", 134217728 },   // 100 mb

        // DBManager
        { "DBManager/Host", "192.168.202.222" },
        { "DBManager/Name", "auto_review_db" },
        { "DBManager/Type", "QPSQL" },
        { "DBManager/Port", 5432 },
        { "DBManager/User", "postgres" },
        { "DBManager/Password", "11" },

        // WebRequestManager
        { "ApiGeneral/Login", "okk" },
    #ifdef QT_DEBUG
        { "ApiGeneral/Password", "ufhe_e7bbbewGhed16t" },
    #else
        { "ApiGeneral/Password", "" },
    #endif
        { "ApiGeneral/UrlImage_1", "http://192.168.211.30:81/api/api_images_taxi_spb.php" },
        { "ApiGeneral/UrlImage_2", "http://192.168.212.30:81/api/api_images_taxi_spb.php" },
        { "ApiGeneral/UrlImage_3", "http://192.168.213.30:81/api/api_images_taxi_spb.php" },
    };
}

bool Core::init()
{
    if (!initLoger())
    {
        qWarning() << "Could not initialize logger";
        return false;
    }

    if (!initServer())
    {
        qWarning() << "Could not initialize server";
        return false;
    }

    if (!initDBManager())
    {
        qWarning() << "Could not initialize dbmanager";
        return false;
    }

    if (!initCommandFactory())
    {
        qWarning() << "Could not initialize command factory";
        return false;
    }

    if (!initWebManager())
    {
        qWarning() << "Could not initialize web manager";
        return false;
    }

    qDebug() << "Core is initialize";
    return true;
}

void Core::done()
{
    _server.reset();
    _logger.reset();
}

bool Core::initLoger()
{
    // TODO: http://134.17.26.128:8080/browse/OKK-125
    return true;

    auto settings = utils::SettingsFactory::instance().settings("server-core");
    settings.beginGroup("Log");

    _logger = QSharedPointer<utils::Logger>::create();

    const auto loggerOptions = QSharedPointer<utils::LoggerMessages::Options>::create();
    loggerOptions->timerInterval = settings["FlushInterval"].toInt();
    if (!_logger->init(loggerOptions))
        return false;

    // FileDevice
    const auto fileOptions = QSharedPointer<utils::FileDevice::FileOptions>::create();
    fileOptions->maxSize = settings["MaxSize"].toLongLong();
    fileOptions->prefixName = settings["PrefixName"].toString();
    fileOptions->directory = settings["Dir"].toString();

    if (!_logger->addDevice(fileOptions))
        return false;

    // DebuggerDevice
    const auto debuggerDevice = QSharedPointer<utils::DebuggerDevice::DebuggerOptions>::create();

    if (!_logger->addDevice(debuggerDevice))
        return false;

    // ConsoleDevice
    const auto consoleDevice = QSharedPointer<utils::ConsoleDevice::ConsoleOptions>::create();

    if (!_logger->addDevice(consoleDevice))
        return false;

    settings.endGroup();
    qDebug() << "initLogger";
    return true;
}

bool Core::initServer()
{
    auto settings = utils::SettingsFactory::instance().settings("server-core");

    _server = core::ServerShp::create();

    core::Server::Options options;
    options.balancerOptions.fastThreadCount = settings.value("MultiThreading/FastThreadCound").toInt();
    options.balancerOptions.longThreadCount = settings.value("MultiThreading/LongThreadCound").toInt();
    options.fastCgiOptions.path = settings.value("FastCGI/ConnectionString").toString().toLatin1();
    options.fastCgiOptions.backlog = settings.value("FastCGI/Backlog").toInt();
    options.fastCgiOptions.maxConnections = settings.value("FastCGI/MaxJobs").toInt();
    options.fastCgiOptions.logTraffic = settings.value("FastCGI/LogTraffic").toBool();

    const auto factory = QSharedPointer<ServerRequestHandlerFactory>::create();
    _server->setHandlerFactory(factory);
    qDebug() << "handler factory is created";

    _server->start(options);
    if (!_server->isStarted())
        return false;

    qDebug() << "initServer";
    return true;
}

bool Core::initDBManager()
{
    auto settings = utils::SettingsFactory::instance().settings("server-core");
    settings.beginGroup("DBManager");

    database::DBManager::DBSettings dbSettings;
    dbSettings.hostName = settings["Host"].toString();
    dbSettings.database = settings["Name"].toString();
    dbSettings.dbType = settings["Type"].toString();
    dbSettings.port = settings["Port"].toInt();
    dbSettings.user = settings["User"].toString();
    dbSettings.password = settings["Password"].toString();
    auto& dbManager = database::DBManager::instance();
    dbManager.initSettings(dbSettings);

    qDebug() << "initDBManager";
    return true;
}

bool Core::initCommandFactory()
{
    qDebug() << "initCommandFactory";
    core::CommandFactory::instance().registerSignatures();
    return true;
}

bool Core::initWebManager()
{
    auto settings = utils::SettingsFactory::instance().settings("server-core");
    settings.beginGroup("ApiGeneral");

    const auto login = settings["Login"].toString();
    const auto password = settings["Password"].toString();
    const auto url = settings["Url"].toString();

    network::WebRequestManager::createSingleton();
    const auto& webManager = network::WebRequestManager::instance();
    webManager->setCredentials(login, password);
    webManager->setUrl(url);

    webManager->init();

    qDebug() << "initWebManager";

    return webManager->isInitialized();
}
