#include <CrLog>


CrLog::CrLog(QObject *object) : QObject(object)
{
    static struct CrLogInit {
        CrLogInit(){ qRegisterMetaType<CrLog::Message>();}
    } init = CrLogInit();
    Q_UNUSED(init);
    messages_.reserve(maxSize_);
}

CrLog::~CrLog()
{

}

int CrLog::maxSize() const
{
    return maxSize_;
}

void CrLog::setMaxSize(int maxSize)
{
    if(maxSize_ == maxSize)
        return;
    maxSize_ = maxSize;
    if( messages_.size() > maxSize_){
        messages_ = messages_.mid(messages_.size() - maxSize_);
    }
    messages_.reserve(maxSize_);
}



void CrLog::addMessage(const CrLog::Message &message)
{
    if(messages_.size() >= maxSize_){
        messages_.removeAt(0);
    }
    messages_.append(message);
    emit messageAdded(message);
}

QList<CrLog::Message> CrLog::messages() const
{
    return messages_;
}

