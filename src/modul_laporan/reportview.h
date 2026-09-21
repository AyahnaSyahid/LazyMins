#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include "report_data.h"
#include "reporttheme.h"

// Forward declarations untuk kelas renderer agar header lebih ringan
class DailySalesRenderer;
class DailyExpenseRenderer;
class RangeSalesRenderer;
class RangeExpenseRenderer;

class ReportView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ReportView(QWidget* parent = nullptr);

    // ---- public API ----
    // Harian
    void showSalesReport(const DailySalesReport& data, const ReportTheme& theme = {});
    void showExpenseReport(const DailyExpenseReport& data, const ReportTheme& theme = {});
    // Periode (range)
    void showRangeSalesReport(const RangeSalesReport& data, const ReportTheme& theme = {});
    void showRangeExpenseReport(const RangeExpenseReport& data, const ReportTheme& theme = {});
    
    void zoomIn();
    void zoomOut();
    void resetZoom();

    // Export to PDF
    bool exportToPdf(const QString& filePath);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QGraphicsScene* m_scene = nullptr;

    // Helper untuk visual halaman
    void addPageShadow();
};