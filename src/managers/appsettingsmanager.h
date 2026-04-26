#pragma once

#include <QVariantMap>

class AppSettingsManager 
{
  public:
    QVariantMap getSettings(const QString& name) const;
    bool saveSettings(const QString& name, const QVariantMap& va);
    QVariant getValue(const QString& name, bool publicOnly = false) const;
    const QString& errorString() const { return m_errorString; }

  private:
    void setErrorString(const QString& es) { m_errorString = es; }
    void resetErrorString() { m_errorString = ""; }
    QString m_errorString;
};