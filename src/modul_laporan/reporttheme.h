#pragma once
#include <QFont>
#include <QColor>

struct ReportTheme {
    // Fonts
    QFont fontCompany   { "Calibri", 14, QFont::Medium };
    QFont fontSub       { "Calibri",  9 };
    QFont fontMeta      { "Calibri",  9 };
    QFont fontTitle     { "Calibri", 12, QFont::Medium };
    QFont fontPeriod    { "Calibri",  9 };
    QFont fontSection   { "Calibri",  8, QFont::Medium };
    QFont fontTableHead { "Calibri",  8, QFont::Medium };
    QFont fontTableBody { "Calibri",  8 };
    QFont fontBold      { "Calibri",  8, QFont::Bold };
    QFont fontSmall     { "Calibri",  7 };

    // Colors
    QColor colorPrimary     { "#1a1a1a" };
    QColor colorSecondary   { "#6b6b6b" };
    QColor colorTertiary    { "#9e9e9e" };
    QColor colorBorder      { "#d0d0d0" };
    QColor colorBorderStrong{ "#888888" };
    QColor colorCardBg      { "#f5f5f5" };
    QColor colorGroupRowBg  { "#efefef" };

    // Badge colors
    QColor colorBadgeDoneBg  { "#e6f4ea" }; QColor colorBadgeDoneFg  { "#1e6b36" };
    QColor colorBadgeProcBg  { "#fff3cd" }; QColor colorBadgeProcFg  { "#7d5a00" };
    QColor colorBadgePendBg  { "#eeeeee" }; QColor colorBadgePendFg  { "#555555" };
    QColor colorBadgeMaterialBg { "#fff3cd" }; QColor colorBadgeMaterialFg { "#7d5a00" };
    QColor colorBadgeOpsBg   { "#e3f0fb" }; QColor colorBadgeOpsFg   { "#0c447c" };
    QColor colorBadgeOtherBg { "#eeeeee" }; QColor colorBadgeOtherFg { "#555555" };

    // Semantic
    QColor colorDanger  { "#c0392b" };
    QColor colorWarning { "#b8740a" };
    QColor colorSuccess { "#1e6b36" };

    // Layout
    qreal pageWidth    = 794.0;   // A4 72dpi ~794px
    qreal marginLeft   = 40.0;
    qreal marginRight  = 40.0;
    qreal contentWidth = pageWidth - marginLeft - marginRight;
    qreal rowHeight    = 20.0;
    qreal sectionGap   = 14.0;
    qreal cardHeight   = 52.0;
    qreal cardGap      =  8.0;
};