#include "reportview.h"

#include <QWheelEvent>
#include <QKeyEvent>
#include <QPrinter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>

#include "reporttheme.h"
#include "report_renderer_base.h"
#include "daily_expense_renderer.h"
#include "daily_sales_renderer.h"

ReportView::ReportView(QWidget* parent)
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

    resetZoom();
}

void ReportView::showSalesReport(const DailySalesReport& data, const ReportTheme& theme)
{
    m_scene->clear();
    addPageShadow();
    DailySalesRenderer renderer(m_scene, data, theme);
    renderer.render(20);
    resetZoom();
}

void ReportView::showExpenseReport(const DailyExpenseReport& data, const ReportTheme& theme)
{
    m_scene->clear();
    addPageShadow();
    DailyExpenseRenderer renderer(m_scene, data, theme);
    renderer.render(20);
    resetZoom();
}

void ReportView::zoomIn()
{
    scale(1.2, 1.2);
}

void ReportView::zoomOut()
{
    scale(1.0 / 1.2, 1.0 / 1.2);
}

void ReportView::resetZoom()
{
    resetTransform();
    // fit A4 width (794px) into viewport dengan padding 20px di tiap sisi
    qreal factor = (viewport()->width() - 40.0) / 794.0;
    if (factor > 0) scale(factor, factor);
}

bool ReportView::exportToPdf(const QString& filePath)
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

void ReportView::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        double factor = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        scale(factor, factor);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void ReportView::keyPressEvent(QKeyEvent* e)
{
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

void ReportView::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    // Opsional: aktifkan resetZoom() jika ingin responsif tiap kali window berubah ukuran
    // resetZoom();
}

void ReportView::addPageShadow()
{
    // Bayangan tipis di belakang halaman
    auto* shadow = m_scene->addRect(
        22, 22, 794, 2000,
        QPen(Qt::NoPen),
        QBrush(QColor(0, 0, 0, 30)));
    shadow->setZValue(-2);

    // Kertas putih utama
    auto* page = m_scene->addRect(
        20, 20, 794, 2000,
        QPen(QColor("#cccccc"), 0.5),
        QBrush(Qt::white));
    page->setZValue(-1);
}