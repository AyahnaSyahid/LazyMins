#ifndef DataStruct_H
#define DataStruct_H

#include <QString>

struct StructProduct {
    QString code, name, desc, qr, ctime, mtime;
    qreal pcost, psell;
    int _id, area;
    
    StructProduct() : _id(-1) {}
};

Q_DECLARE_METATYPE(StructProduct)

#endif