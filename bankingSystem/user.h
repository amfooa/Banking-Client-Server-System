#ifndef USER_H
#define USER_H

#include <QString>

using namespace std;

class User
{
protected:
    QString m_username;
    QString m_password;
    bool connectedToServer;

public:
    User(){};
    virtual bool login(QString &m_username, QString &m_password) = 0;
    virtual int viewAccountBalance(QString m_accountNumber) = 0;
    virtual QString viewTransactionHistory(QString m_accountNumber, int count) = 0;
    ~User(){};
};

#endif // USER_H
