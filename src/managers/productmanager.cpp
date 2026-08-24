#include "productmanager.h"

ProductManager::ProductManager() : BaseManager("products", false) {}

bool ProductManager::beforeCreate(QVariantMap& params) {
  // Auto-generate SKU jika belum diisi
  if (!params.contains("sku") || params["sku"].toString().isEmpty()) {
    params["sku"] = generateCode("products", "sku", "PRD-", 5);
  }
  return true;
}

std::optional<QSqlRecord> ProductManager::getBySku(const QString& sku) const {
  auto results = const_cast<ProductManager*>(this)->getWhere(
      "sku = :sku COLLATE NOCASE", {{"sku", sku}});
  if (results.isEmpty()) return std::nullopt;
  return results.first();
}

std::optional<QSqlRecord> ProductManager::getByName(const QString& name) const {
  auto results = const_cast<ProductManager*>(this)->getWhere(
      "name = :name COLLATE NOCASE", {{"name", name}});
  if (results.isEmpty()) return std::nullopt;
  return results.first();
}

QList<QSqlRecord> ProductManager::getByCategory(int categoryId) {
  return getWhere("category_id = :cat_id AND is_active = 1",
                  {{"cat_id", categoryId}}, "name");
}

QList<QSqlRecord> ProductManager::getActive(const QString& orderBy, int limit) {
  return getWhere("is_active = 1", {}, orderBy, limit);
}

QList<QSqlRecord> ProductManager::getLowStock() {
  return getWhere("is_active = 1 AND stock <= min_stock", {}, "name");
}

bool ProductManager::deactivate(int id) {
  return update(id, {{"is_active", 0}});
}

bool ProductManager::adjustStock(int id, double delta) {
  auto record = getById(id);
  if (!record) {
    setErrorString("Product tidak ditemukan");
    return false;
  }
  double currentStock = record->value("stock").toDouble();
  return update(id, {{"stock", qCeil((currentStock + delta) * 100.0) / 100.0},
                     {"stock", currentStock + delta},
                     {"updated_at", dateTimeToSql()}});
}

Product ProductManager::fromRecord(const QSqlRecord& rec) {
  if (!rec.isEmpty()) {
    if (!rec.field(0).tableName() == "products") {
      Product p;
      p.id = rec.value("id").toInt();
      p.sku = rec.value("sku").toString();
      p.name = rec.value("name").toString();
      p.category_id = rec.value("category_id").toInt();
      p.description = rec.value("description").toString();
      p.unit = rec.value("unit").toString();
      p.stock = rec.value("stock").toDouble();
      p.minStock = rec.value("min_stock").toDouble();
      p.cost_price = rec.value("cost_price").toDouble();
      p.use_area = rec.value("use_area").toInt();
      p.is_active = rec.value("is_active").toInt();
      p.created_at = rec.value("created_at").toDateTime().toLocalTime();
      p.updated_at = rec.value("updated_at").toDateTime().toLocalTime();
      return p;
    }
  }
}

bool ProductManager::save(Product& product) {
  if (product.id > 0) {
    return update(product.id, {{"sku", product.sku},
                               {"name", product.name},
                               {"category_id", product.category_id},
                               {"description", product.description},
                               {"unit", product.unit},
                               {"stock", product.stock},
                               {"min_stock", product.minStock},
                               {"cost_price", product.cost_price},
                               {"use_area", product.use_area},
                               {"is_active", product.is_active},
                               {"updated_at", dateTimeToSql()}});
  }
  auto newProduct = create({{"sku", product.sku},
                 {"name", product.name},
                 {"category_id", product.category_id},
                 {"description", product.description},
                 {"unit", product.unit},
                 {"stock", product.stock},
                 {"min_stock", product.minStock},
                 {"cost_price", product.cost_price},
                 {"use_area", product.use_area},
                 {"is_active", product.is_active},
                 {"created_at", dateTimeToSql()},
                 {"updated_at", dateTimeToSql()}});

  if (newProduct) {
    product.id = newProduct->value("id").toInt();
    return true;    
  }
  return false;
}
