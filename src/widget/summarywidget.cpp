#include "summarywidget.h"
#include "ui_summarywidget.h"
#include "../databaseinterface.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

SummaryWidget::SummaryWidget(QWidget *parent) 
: ui(new Ui::SummaryWidget), m_topLevel(nullptr), RealTimeDataWIdget(parent)
{
  ui->setupUi(this);
  
}

SummaryWidget::~SummaryWidget()  { delete ui; }

void SummaryWidget::initTopLevel()
{
  if (m_topLevel) {
    auto d = m_topLevel;
    d->deleteLater();
  }
  m_topLevel = new QWidget;
  auto topLayout = new QVBoxLayout(m_topLevel);
  auto 
}

QWidget* SummaryWidget::createStatCard(const QString &title, const QString &value, const QString &subtext, const QString &color) {
    auto *card = new QFrame();
    card->setObjectName("WhiteCard");
    card->setAutoFillBackground(true);
    
    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(2);

    auto *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("font-size: 10px; font-weight: 800; color: #95a5a6;");
    
    auto *lblValue = new QLabel(value);
    lblValue->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1;").arg(color));

    auto *lblSub = new QLabel(subtext);
    lblSub->setStyleSheet("font-size: 11px; color: #7f8c8d; italic;");

    layout->addWidget(lblTitle);
    layout->addWidget(lblValue);
    layout->addWidget(lblSub);

    // Efek Shadow halus (Opsional, Qt6 mendukung efek ini dengan baik)
    auto *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 120));
    card->setGraphicsEffect(shadow);

    return card;
}

void SummaryWidget::reloadModelData(const QStringList& tn)
{
  if ( tn.contains("invoices") ||
       tn.contains("payments") ) {
    auto &di = DatabaseInterface::instance();
    auto &db = di.database();
    
    QSqlQuery q(db);
  }
}