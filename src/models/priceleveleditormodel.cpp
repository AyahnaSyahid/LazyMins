#include "priceleveleditormodel.h"
#include "src/managers/managers.h"

PriceLevelEditorModel::PriceLevelEditorModel(QObject *p)
: QAbstractTableModel(p)
{
  
}

PriceLevelEditorModel::~PriceLevelEditorModel() {}

bool setProductId(int id)
