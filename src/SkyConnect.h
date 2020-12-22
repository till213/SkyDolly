#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include <windows.h>
#include <QObject>

class SkyConnect : public QObject
{
    Q_OBJECT
public:
    SkyConnect();
    virtual ~SkyConnect();

    bool open();
    bool close();
    bool isConnected() const;

private:
    HANDLE m_simConnectHandler;
};

#endif // SKYCONNECT_H
