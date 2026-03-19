#pragma once

#include <QVariantMap>

class AppSettingsManager 
{
  public:
    QVariantMap getSettings(const QString& name) const;
    bool saveSettings(const QString& name, const QVariantMap& va) const;
};