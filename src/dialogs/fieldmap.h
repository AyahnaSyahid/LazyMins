#pragma once

#include <QLineEdit>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QString>
#include <variant>

/**
 * FieldMap
 * Maps a widget editor to a database column key.
 * Add more widget types to the variant as needed.
 */
struct FieldMap {
    std::variant<QLineEdit*, QSpinBox*, QDoubleSpinBox*, QPlainTextEdit*> editor;
    QString key;
};
