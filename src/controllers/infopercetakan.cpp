#include "infopercetakan.h"
#include "src/managers/appsettingsmanager.h"
#include "src/managers/basemanager.h"
#include "src/utils/sessionmanager.h"
#include <QSqlError>
#include <QSqlQuery>

InfoPercetakan InfoPercetakanController::getInfoPercetakan() const
{
    AppSettingsManager apm;
    InfoPercetakan info;
    info.nama = apm.getValue("company_name").toString().toStdString();
    info.telp = apm.getValue("company_phone").toString().toStdString();
    info.email = apm.getValue("company_email").toString().toStdString();
    info.alamat = apm.getValue("company_address").toString().toStdString();
    return info;
}

bool InfoPercetakanController::saveInfoPercetakan(const InfoPercetakan &info, std::string *error)
{
    int currentUserId = SessionManager::instance().currentUserId();
    QSqlQuery q(BaseManager::connection);
    QStringList keys = {"company_name", "company_phone", "company_email", "company_address"};
    QStringList newValues = {QString::fromStdString(info.nama),
                             QString::fromStdString(info.telp),
                             QString::fromStdString(info.email),
                             QString::fromStdString(info.alamat)};
    for (int i = 0; i < keys.size(); ++i)
    {
        q.prepare("UPDATE app_settings SET setting_value = :val, updated_by = :upby, updated_at = :upat WHERE setting_key = :key");
        q.bindValue(":val", newValues[i]);
        q.bindValue(":key", keys[i]);
        q.bindValue(":upby", currentUserId);
        q.bindValue(":upat", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss"));
        if (!q.exec())
        {
            if (error)
                *error = q.lastError().text().toStdString();
            return false;
        }
    }
    return true;
}