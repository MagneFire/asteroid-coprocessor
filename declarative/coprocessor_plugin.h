#ifndef COPROCESSOR_PLUGIN_H
#define COPROCESSOR_PLUGIN_H

#include <QQmlExtensionPlugin>

class CoprocessorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.asteroid.coprocessor")

public:
    explicit CoprocessorPlugin(QObject *parent = 0);

    virtual void registerTypes(const char *uri);
};

#endif // COPROCESSOR_PLUGIN_H

