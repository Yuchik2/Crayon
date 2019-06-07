#ifndef CRLOG_H
#define CRLOG_H

#include "crayoncore_global.h"
#include <QObject>
#include <QList>
#include <QDateTime>

class CrNode;
class CRAYON_CORE_EXPORT CrLog : public QObject
{
    Q_OBJECT
public:
    enum MessageType{
        Info,
        Warning,
        Error
    };
    Q_ENUM(MessageType)

    struct Message
    {
        Message(const QString& title = QString(), const QString &text = QString(), MessageType type = Info, CrNode* node = nullptr) :
            title(title), text(text), type(type), node(node), time_(QDateTime::currentDateTime()){}

        QString title;
        QString text;
        MessageType type;
        CrNode* node;
        QDateTime time_;
    };

    explicit CrLog(QObject* object = nullptr);
    virtual ~CrLog() override;

    int maxSize() const;
    void setMaxSize(int maxSize);

    QList<Message> messages() const;

    void addMessage(const Message& message);
    void addMessage(const QString& title, const QString &text = QString(), MessageType type = Info, CrNode* node = nullptr)
    {
        Message ms{title, text, type, node};
        addMessage(ms);
    }

    void addInfoMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr)
    {
        Message ms{title, text, Info, node};
        addMessage(ms);
    }

    void addWarningMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr)
    {
        Message ms{title, text, Warning, node};
        addMessage(ms);
    }

    void addErrorMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr)
    {
        Message ms{title, text, Error, node};
        addMessage(ms);
    }

signals:
    void messageAdded(CrLog::Message message);

private:
    Q_DISABLE_COPY(CrLog)

    QList<Message> messages_;
    int maxSize_ = 2048;
};

Q_DECLARE_METATYPE(CrLog::Message)

#endif // CRLOG_H
