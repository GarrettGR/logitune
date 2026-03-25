#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "DeviceManager.h"
#include "models/DeviceModel.h"
#include "models/ButtonModel.h"
#include "models/ActionModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Logitune");
    app.setApplicationName("Logitune");

    // Create backend
    logitune::DeviceManager deviceManager;
    logitune::DeviceModel deviceModel;
    deviceModel.setDeviceManager(&deviceManager);

    logitune::ButtonModel buttonModel;
    logitune::ActionModel actionModel;

    QQmlApplicationEngine engine;

    // Register singletons — provide existing instances
    qmlRegisterSingletonInstance("Logitune", 1, 0, "DeviceModel", &deviceModel);
    qmlRegisterSingletonInstance("Logitune", 1, 0, "ButtonModel", &buttonModel);
    qmlRegisterSingletonInstance("Logitune", 1, 0, "ActionModel", &actionModel);

    engine.loadFromModule("Logitune", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    // Start device monitoring after QML is loaded
    deviceManager.start();

    return app.exec();
}
