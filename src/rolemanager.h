#ifndef RoleManagerH
#define RoleManagerH

#include <QString>
#include <QSqlRecord>
#include <QVariant>

struct Role {
    qint64 role_id;
    QString role_name, description;
    // Role() : role_id(-1) {}
    static Role fromRecord(const QSqlRecord& rec) {
        Role r;
        r.role_id = rec.value("role_id").toLongLong();
        r.role_name = rec.value("role_name").toString();
        r.description = rec.value("description").toString();
        return r;
    }
};

class RoleManager {
    public:
        bool createRole(const QString& name, const QString& desc=QString(), Role* ref=nullptr);
        bool roleExists(const QString& name);
        Role getRole(qint64 role_id) const;
        Role removeRole(qint64 role_id);
};

#endif //RoleManagerH