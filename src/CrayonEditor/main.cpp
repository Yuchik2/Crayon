#include "QApplication"

#include "crgraphicseditor.h"
#include <CrCore>
#include <QDebug>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{  
    QApplication appliaction(argc, argv);

    CrCore::installTranslator(QLibraryInfo::location(QLibraryInfo::TranslationsPath), QLocale::system());

    CrCore::addPluginsPath(QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath));

    CrGraphicsEditor* editor = new CrGraphicsEditor();
    editor->show();
    return appliaction.exec();
}
