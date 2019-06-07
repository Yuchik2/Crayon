#ifndef CRCORE_H
#define CRCORE_H

#include "crayoncore_global.h"
#include <QObject>

class CrPlugin;

class CrCorePrivate;
class CRAYON_CORE_EXPORT CrCore : public QObject
{
    Q_OBJECT
public:
    enum Mode{
        Editor,
        Monitor,
    };
    Q_ENUM(Mode)

    static CrCore* instance();

    static Mode mode();
    static void setMode(Mode mode);

    static void addPluginsPath(const QString& path);
    static QList<CrPlugin*> plugins();

    static void installTranslator(const QString& path, const QLocale& locale);

signals:
    void modeChanged();
    void pluginsChanged();
    void translatorsChanged();
    void errorOccured(const QString& errorString);

private:
    explicit CrCore();
    virtual ~CrCore() override;
    CrCorePrivate* d_;
};

#endif // CRCORE_H
