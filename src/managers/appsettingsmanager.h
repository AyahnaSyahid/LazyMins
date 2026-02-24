#pragma once

#include <QVariantMap>

class AppSettingsManager 
{
  public:
    QVariantMap getSettings(const QString& name) const;
};