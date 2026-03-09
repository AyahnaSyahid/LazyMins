#pragma once
#include <QVariantMap>

void debugMap(const QVariantMap& map) {
  qDebug().noquote() << "==============================";
  for(auto const &[key, value] : map.asKeyValueRange())
    qDebug().noquote() << "  " << key << " : " << value.toString();
  qDebug().noquote() << "==============================";
}