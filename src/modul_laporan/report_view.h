#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPrinter>
#include <QPainter>

#include "report_data.h"
#include "daily_sales_renderer.h"
#include "daily_expense_renderer.h"

// ============================================================
//  ReportView
//  Drop-in QWidget — plug into any layout.
//
//  Usage:
//    auto* view = new ReportView(parent);
//    view->showSalesReport(data);
//    view->showExpenseReport(data);
//    view->exportToPdf("/tmp/laporan.pdf");
// ============================================================

class ReportView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ReportView(QWidget* parent = nullptr)
        : QGraphicsView(parent)
    {
        m_scene = new QGraphicsScene(this);
        setScene(m_scene);

        setRenderHint(QPainter::Antialiasing);
        setRenderHint(QPainter::TextAntialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
        setBackgroundBrush(QBrush(QColor("#e8e8e8")));
        setFrameShape(QFrame::NoFrame);

        // default scale — fit A4 width
        resetZoom();
    }

    // ---- public API ----

    void showSalesReport(const DailySalesReport& data,
                         const ReportTheme& theme = {})
    {
        m_scene->clear();
        addPageShadow();
        DailySalesRenderer renderer(m_scene, data, theme);
        renderer.render(20);
        resetZoom();
    }

    void showExpenseReport(const DailyExpenseReport& data,
                           const ReportTheme& theme = {})
    {
        m_scene->clear();
        addPageShadow();
        DailyExpenseRenderer renderer(m_scene, data, theme);
        renderer.render(20);
        resetZoom();
    }

    void zoomIn()  { scale(1.2, 1.2); }
    void zoomOut() { scale(1.0 / 1.2, 1.0 / 1.2); }

    void resetZoom() {
        resetTransform();
        // fit A4 width (794px) into viewport with 20px padding each side
        qreal factor = (viewport()->width() - 40.0) / 794.0;
        if (factor > 0) scale(factor, factor);
    }

    // Export to PDF (requires Qt PrintSupport module)
    bool exportToPdf(const QString& filePath)
    {
        if (m_scene->items().isEmpty()) return false;

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageOrientation(QPageLayout::Portrait);

        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        m_scene->render(&painter);
        painter.end();
        return true;
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        if (e->modifiers() & Qt::ControlModifier) {
            double factor = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
            scale(factor, factor);
            e->accept();
        } else {
            QGraphicsView::wheelEvent(e);
        }
    }

    void keyPressEvent(QKeyEvent* e) override {
        if (e->modifiers() & Qt::ControlModifier) {
            if (e->key() == Qt::Key_Equal || e->key() == Qt::Key_Plus)
                zoomIn();
            else if (e->key() == Qt::Key_Minus)
                zoomOut();
            else if (e->key() == Qt::Key_0)
                resetZoom();
        }
        QGraphicsView::keyPressEvent(e);
    }

    void resizeEvent(QResizeEvent* e) override {
        QGraphicsView::resizeEvent(e);
        // optional: re-fit on resize
        // resetZoom();
    }

private:
    QGraphicsScene* m_scene = nullptr;

    // White page card behind the report
    void addPageShadow() {
        // light shadow rect
        auto* shadow = m_scene->addRect(
            22, 22, 794, 2000,
            QPen(Qt::NoPen),
            QBrush(QColor(0, 0, 0, 30)));
        shadow->setZValue(-2);

        // white page
        auto* page = m_scene->addRect(
            20, 20, 794, 2000,
            QPen(QColor("#cccccc"), 0.5),
            QBrush(Qt::white));
        page->setZValue(-1);
    }
};
