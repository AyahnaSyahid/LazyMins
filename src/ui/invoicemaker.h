#ifndef InvoiceMaker_H
#define InvoiceMaker_H

class QImage;
class QDialog;
class InvoiceMaker:
public:
  InvoiceMaker(int invid);
  ~InvoiceMaker();
private:
  QImage *img;
};

#endif