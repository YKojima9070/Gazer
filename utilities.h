#ifndef UTILITIES_H
#define UTILITIES_H
#include <QString>

#endif // UTILITIES_H

class Utilities{

public:
    static QString getDataPath();
    static QString newSavedVideoName();
    static QString getSavedVideoPath(QString name, QString postfix);

};
