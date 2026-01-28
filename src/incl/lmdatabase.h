#ifndef LMDatabase_H
#define LMDatabase_H

#include <QObject>

class LMDatabase : public QObject
{
  Q_OBJECT
  public:
    LMDatabase(QObject *parent = nullptr);
    ~LMDatabase();
    
    bool transaction();
    bool commit();
    bool rollback();
    
    
};

#endif