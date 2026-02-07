#ifndef AdminManager_H
#define AdminManager_H

#include <QSqlRecord>

struct CreateAdminParams {
  // just required field
  int id,
      role_id;
  QString username,
          literal_password,
          nama_lengkap,
          email,
          nomor_telepon;
};

class AdminManager
{
  public:
    QSqlRecord create(CreateAdminParams &pa);
    QSqlRecord getById(int id);
    QSqlRecord getByUsername(const QString &name);
    bool remove(int id);
    bool setRole(int id, int role);
    bool setStatus(int id, int state);
    bool save(const QSqlRecord&);
    void updateLastLogin(int id);
};

#endif