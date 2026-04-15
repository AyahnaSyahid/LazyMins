#ifndef ESC_POS_H
#define ESC_POS_H

class EscPosBuilder {
public:
    void initializePrinter();
    void setFontSize(int size);
    void printText(const char* text);
    void printBarcode(const char* barcodeData, const char* type);
    void printQRCode(const char* data);
    // Additional methods ...
};

class EscPosPrinter {
public:
    void openConnection(const char* port);
    void closeConnection();
    void sendCommand(const char* command);
    // Additional communication methods ...
};

#endif // ESC_POS_H
