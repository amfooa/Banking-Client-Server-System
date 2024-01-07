#ifndef ADMIN_H
#define ADMIN_H

#include "user.h"

class Admin : public User
{
private:

public:
    Admin();
    virtual bool login(QString &m_username, QString &m_password);
    virtual int viewAccountBalance(QString m_accountNumber);
    virtual QString viewTransactionHistory(QString m_accountNumber, int count);

    QString getAccountNumber(QString adminName, QString m_username);
    QString viewBankDB(void);
    bool createUser(QString userData);
    bool deleteUser(QString m_accountNumber);
    bool updateUser(QString m_accountNumber, QString newData);
};

#endif // ADMIN_H
