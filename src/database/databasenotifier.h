#ifndef DatabaseNotifierH
#define DatabaseNotifierH

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

class DatabaseNotifier : public QObject {
    Q_OBJECT
    
    DatabaseNotifier(QObject* parent=nullptr) : QObject(parent) {}
public:
    
    static DatabaseNotifier* instance() {
        QMutexLocker (&DatabaseNotifier::mutex);
        
        if(!_inst) {
            _inst = new DatabaseNotifier;
        }
        return _inst;
    }
    
    DatabaseNotifier(const DatabaseNotifier&) = delete;
    DatabaseNotifier& operator=(const DatabaseNotifier&) = delete;

public slots:
    inline void emitChanged(const QString& s="") {
        emit tableChanged(s);
    }

signals:
    tableChanged(const QString& s="");

private:
    static QMutex mutex;
    static DatabaseNotifier* _inst;
};

#define dbNot DatabaseNotifier::instance()

#endif // DatabaseNotifierH