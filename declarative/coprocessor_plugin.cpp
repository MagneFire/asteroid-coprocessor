#include "coprocessor_plugin.h"
#include <QtQml>
#include <Coprocessor>
#include <Hands>

CoprocessorPlugin::CoprocessorPlugin(QObject *parent) : QQmlExtensionPlugin(parent)
{
}

void CoprocessorPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<Coprocessor>(uri, 1,0, "Coprocessor");
    qmlRegisterType<Hands>(uri, 1,0, "Hands");
}

