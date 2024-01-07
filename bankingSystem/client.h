#ifndef Client_H
#define Client_H

#include "user.h"


class Client : public User
{
private:
    short m_age;
    int m_balance;
    QString m_accountNumber;
    QString m_fullName;

public:
    Client();

    //Admin & Client Functions
    virtual bool login(QString &m_username, QString &m_password);
    virtual int viewAccountBalance(QString m_accountNumber);
    virtual QString viewTransactionHistory(QString m_accountNumber, int count);


    //Client Specific Functions
    QString getAccountNumber(QString m_username);
    bool makeTransaction(QString m_accountNumber, signed int transactionAmount);
    bool transferAmount(QString fromAccountNumber, QString toAccountNumber, unsigned int transactionAmount);

};

#endif // Client_H
