#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <iostream>


#include "client.h"
#include "admin.h"

using namespace std;

#define MAX_LOGIN_ATTEMPTS 3


// Function to clear the console (works on many platforms but might not be portable)
void clearConsole() {
// Use the system command to clear the console
// Note: This method might not work on all platforms
#ifdef _WIN32
    system("cls"); // For Windows
#else
    system("clear"); // For Unix-like systems
#endif
}


void printTable(const QVector<QVector<QString>> &table) {
    QTextStream out(stdout);

    // Find the maximum width for each column
    QVector<int> columnWidths(table[0].size(), 0);
    for (const auto &row : table) {
        for (int col = 0; col < row.size(); ++col) {
            columnWidths[col] = qMax(columnWidths[col], row[col].length());
        }
    }

    // Print the table header
    for (int col = 0; col < table[0].size(); ++col) {
        int padding = (columnWidths[col] - table[0][col].length()) / 2;
        out << QString("%1").arg("", padding) << table[0][col]
            << QString("%1").arg("", columnWidths[col] + 2 - padding - table[0][col].length());
    }
    out << "\n";

    // Print the table content
    for (int row = 1; row < table.size(); ++row) {
        for (int col = 0; col < table[row].size(); ++col) {
            out << QString("%1").arg(table[row][col], -columnWidths[col] - 2);
        }
        out << "\n";
    }
}

bool loginStatus = false;
short loginAttempts = 0;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    string tempUsername;
    string tempPassword;
    string tempAccountNumber;
    string tempFullName;
    string tempBalance;
    string tempAge;
    string tempPrivilege;
    QString tempUserData;
    QString tempHistory;

    QStringList dataSV;
    QVector<QVector<QString>> myTable;

    int tempIntBalance;
    int tempCount;


    char tempInput;

    cout << "Welcome to Banking System" << endl << endl;
    cout << "For Staff Login: 1\nFor Customers Login: 2" << endl << endl;
    cout << "Choice: ";
    cin  >> tempInput;

    while (tempInput != '1' && tempInput != '2')
    {
        cout << "Invalid Input";
        QThread::sleep(2);
        clearConsole();
        cout << "For Staff Login: 1\nFor Customers Login: 2\n";
        cin  >> tempInput;
    }

    if(tempInput == '1')
    {
        Admin tempAdmin;

        QString authUsername;
        QString authPassword;

        do
        {
            cout << "Login:-" << endl;

            cout << "Enter Username: " ;
            cin>>tempUsername;

            cout << "Enter Password: ";
            cin  >> tempPassword;

            // // Send data to the server
            authUsername = QString::fromStdString(tempUsername);
            authPassword = QString::fromStdString(tempPassword);

            loginStatus = tempAdmin.login(authUsername, authPassword);

            if(loginStatus != true)
            {
                cout << "Invalid Credentials" << 2-loginAttempts <<" Attempts Left";
                loginAttempts++;
                QThread::sleep(2);
                clearConsole();
            }
            else
            {
                clearConsole();
                cout <<"Login Successful" << endl;
                QThread::sleep(2);
                clearConsole();
            }
        }while(!loginStatus && loginAttempts < MAX_LOGIN_ATTEMPTS);

        if(loginAttempts == MAX_LOGIN_ATTEMPTS)
        {
            cout << "Max Login Attempts Reached, Exiting...";
            exit(0);
        }
        else
        {
            for(;;)
            {
                cout << "Services: "                  << endl;
                cout << "1- Get Account Number"       << endl;
                cout << "2- View Account Balance"     << endl;
                cout << "3- View Transaction History" << endl;
                cout << "4- View Bank Database"       << endl;
                cout << "5- Create New User"          << endl;
                cout << "6- Delete User"              << endl;
                cout << "7- Update User"              << endl;
                cout << "8- Exit"       << endl       << endl;

                cin >> tempInput;

                switch(tempInput)
                {
                case '1':
                    clearConsole();
                    cout << "Enter Username: ";
                    cin  >> tempUsername;

                    tempAccountNumber = tempAdmin.getAccountNumber(authUsername, QString::fromStdString(tempUsername)).toStdString();

                    if(tempAccountNumber != "NaN")
                    {
                        cout << "Account Number: " <<tempAccountNumber << endl << endl;
                    }
                    else
                    {
                        cout << "Username Doesn't Exist" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    break;
                case '2':
                    clearConsole();
                    cout << "Enter Account Number: ";
                    cin  >> tempAccountNumber;

                    tempIntBalance = tempAdmin.viewAccountBalance(QString::fromStdString(tempAccountNumber));

                    if(tempIntBalance != 0)
                    {
                        cout << "Account Balance: " << tempIntBalance << endl << endl;
                    }
                    else
                    {
                        cout << "Account Balance Doesn't Exist" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }


                    break;
                case '3':
                    clearConsole();
                    cout << "Enter Account Number: ";
                    cin  >> tempAccountNumber;

                    cout << "Count: ";
                    cin  >> tempCount;

                    tempHistory = tempAdmin.viewTransactionHistory(QString::fromStdString(tempAccountNumber), tempCount);


                    if(tempHistory != "none")
                    {
                        tempHistory = "Account Number,Amount,Date|" + tempHistory;
                        dataSV = tempHistory.split('|');
                        cout << "Transaction History:" << endl << endl;


                        for (const QString &row : dataSV) {
                            QStringList columns = row.split(',');
                            myTable.push_back(columns.toVector());
                        }

                        printTable(myTable);

                        myTable.clear();

                    }
                    else
                    {
                        cout << "Account Number Doesn't Exist" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    break;
                case '4':
                    clearConsole();
                    tempHistory = tempAdmin.viewBankDB();



                    if(tempHistory != "none")
                    {
                        tempHistory = "Username,Account Number,Full Name,Balance,Age|" + tempHistory;
                        dataSV = tempHistory.split('|');
                        cout << "Bank Database:" << endl << endl;

                        for (const QString &row : dataSV) {
                            QStringList columns = row.split(',');
                            myTable.push_back(columns.toVector());
                        }

                        printTable(myTable);

                        myTable.clear();

                    }
                    else
                    {
                        cout << "Database Empty" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }

                    break;
                case '5':
                    clearConsole();

                    cout << "Enter User Type:" << endl << "1: Admin" << endl << "2: Client" << endl << endl << "Choice";
                    cin >> tempPrivilege;

                    if(tempPrivilege == "1")
                    {
                        cout << "Enter Username: ";
                        cin >> tempUsername;

                        cout << "Enter Password: ";
                        cin >> tempPassword;

                        tempUserData = QString::fromStdString(tempPrivilege + ',' + tempUsername + ',' + tempPassword);
                        if(tempAdmin.createUser(tempUserData))
                        {
                            cout << "Admin Added Successfully" << endl;
                            QThread::sleep(2);
                            clearConsole();
                        }
                        else
                        {
                            cout << "Failed to Add Admin" << endl;
                            QThread::sleep(2);
                            clearConsole();
                        }
                    }
                    else if(tempPrivilege == "2")
                    {
                        cout << "Enter Username: ";
                        cin >> tempUsername;

                        cout << "Enter Password: ";
                        cin >> tempPassword;

                        cout << "Enter Account Number: ";
                        cin >> tempAccountNumber;

                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                        cout << "Enter Full Name: ";
                        getline(std::cin, tempFullName);

                        cout << "Enter Balance: ";
                        cin >> tempBalance;

                        cout << "Enter Age: ";
                        cin >> tempAge;

                        tempUserData = QString::fromStdString(tempPrivilege + ',' + tempUsername + ',' + tempPassword + ',' + tempAccountNumber + ',' +  tempFullName + ',' +  tempBalance + ',' +  tempAge);
                        if(tempAdmin.createUser(tempUserData))
                        {
                            cout << "Client Added Successfully" << endl;
                            QThread::sleep(2);
                            clearConsole();
                        }
                        else
                        {
                            cout << "Failed to Add Client" << endl;
                            QThread::sleep(2);
                            clearConsole();
                        }
                    }
                    else
                    {
                        cout << "Invalid Input" << endl;

                        QThread::sleep(2);

                        clearConsole();

                    }
                    break;
                case '6':
                    clearConsole();
                    cout << "Enter Account Number: ";
                    cin  >> tempAccountNumber;


                    if(tempAdmin.deleteUser(QString::fromStdString(tempAccountNumber)))
                    {
                        cout << "Account Deleted" << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    else
                    {
                        cout << "Account Doesn't Exist" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    break;
                case '7':
                    cout << "Enter Username: ";
                    cin >> tempUsername;

                    cout << "Enter Password: ";
                    cin >> tempPassword;

                    cout << "Enter Account Number: ";
                    cin >> tempAccountNumber;

                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    cout << "Enter Full Name: ";
                    getline(std::cin, tempFullName);

                    cout << "Enter Balance: ";
                    cin >> tempBalance;

                    cout << "Enter Age: ";
                    cin >> tempAge;

                    tempUserData = QString::fromStdString(tempUsername + ',' + tempPassword + ',' +  tempFullName + ',' +  tempBalance + ',' +  tempAge);
                    if(tempAdmin.updateUser(QString::fromStdString(tempAccountNumber), tempUserData))
                    {
                        cout << "Client Updated Successfully" << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    else
                    {
                        cout << "Failed to Update Client" << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }



                    break;
                case '8':
                    cout << "Exiting...";
                    QThread::sleep(2);
                    exit(0);
                    break;
                default:
                    cout << "Invalid Service ID" << endl;
                    QThread::sleep(2);
                    clearConsole();
                    break;
                }
            }
        }


    }
    else
    {
        Client tempClient;


        QString authUsername;
        QString authPassword;

        do
        {
            cout << "Login:-" << endl;

            cout << "Enter Username: " ;
            cin>>tempUsername;

            cout << "Enter Password: ";
            cin  >> tempPassword;

            // // Send data to the server
            authUsername = QString::fromStdString(tempUsername);
            authPassword = QString::fromStdString(tempPassword);

            loginStatus = tempClient.login(authUsername, authPassword);

            if(loginStatus != true)
            {
                cout << "Invalid Credentials" << 2-loginAttempts <<" Attempts Left";
                loginAttempts++;
                QThread::sleep(2);
                clearConsole();
            }
            else
            {
                clearConsole();
                cout <<"Login Successful" << endl;
                QThread::sleep(2);
                clearConsole();
            }
        }while(!loginStatus && loginAttempts < MAX_LOGIN_ATTEMPTS);

        if(loginAttempts == MAX_LOGIN_ATTEMPTS)
        {
            cout << "Max Login Attempts Reached, Exiting...";
            exit(0);
        }
        else
        {
            for(;;)
            {
                cout << "Services: "                  << endl;
                cout << "1- Get Account Number"       << endl;
                cout << "2- View Account Balance"     << endl;
                cout << "3- View Transaction History" << endl;
                cout << "4- Make A Transaction"       << endl;
                cout << "5- Transfer Amount"          << endl;
                cout << "6- Exit"       << endl       << endl;

                cin >> tempInput;

                switch(tempInput)
                {
                case '1':
                    clearConsole();
                    cout << "Account Number: " << tempClient.getAccountNumber(authUsername).toStdString() << endl << endl;
                    break;
                case '2':
                    clearConsole();
                    cout << "Account Balance: " << tempClient.viewAccountBalance(tempClient.getAccountNumber(authUsername)) << endl << endl;
                    break;
                case '3':
                    clearConsole();

                    cout << "Count: ";
                    cin  >> tempCount;

                    tempHistory = tempClient.viewTransactionHistory(QString::fromStdString(tempClient.getAccountNumber(authUsername).toStdString()), tempCount);


                    if(tempHistory != "none")
                    {
                        qDebug() << tempHistory;
                    }
                    else
                    {
                        cout << "Account Number Doesn't Exist" << endl << endl;
                        QThread::sleep(2);
                        clearConsole();
                    }
                    break;
                case '4':
                    clearConsole();
                    cout <<"Make A Transaction:" << endl << endl;
                    cout <<"1: Deposit" << endl << "2: Withdraw" << endl << endl;
                    cout <<"Choice: " ;

                    cin >> tempInput;

                    if(tempInput == '1')
                    {
                        cout << endl << "Enter Amount: ";
                        cin  >> tempIntBalance;

                        tempAccountNumber = tempClient.getAccountNumber(authUsername).toStdString();
                        if(tempClient.makeTransaction(QString::fromStdString(tempAccountNumber), tempIntBalance >= 0 ? tempIntBalance : 0))
                        {
                            cout << "Successful Transaction";
                            QThread::sleep(2);
                            clearConsole();
                        }
                        else
                        {
                            cout << "Failed Transaction";
                            QThread::sleep(2);
                            clearConsole();
                        }
                    }
                    else if(tempInput == '2')
                    {
                        cout << endl << "Enter Amount: ";
                        cin  >> tempIntBalance;

                        tempAccountNumber = tempClient.getAccountNumber(authUsername).toStdString();
                        if(tempClient.makeTransaction(QString::fromStdString(tempAccountNumber), tempIntBalance > 0 ? (tempIntBalance * -1) : tempIntBalance))
                        {
                            cout << "Successful Transaction";
                            QThread::sleep(2);
                            clearConsole();
                        }
                        else
                        {
                            cout << "Failed Transaction";
                            QThread::sleep(2);
                            clearConsole();
                        }
                    }
                    else
                    {
                        cout << "Invalid Operation";
                        QThread::sleep(2);
                        clearConsole();
                    }

                    break;
                case '5':
                    clearConsole();
                    cout <<"Transfer Amount:" << endl << endl;
                    cout <<"Destination Account Number: " ;
                    cin  >> tempAccountNumber;
                    cout <<"Amount: " ;
                    cin  >> tempIntBalance;

                    if(tempClient.transferAmount(tempClient.getAccountNumber(authUsername), QString::fromStdString(tempAccountNumber), tempIntBalance))
                    {
                        cout << "Successful Transaction";
                        QThread::sleep(2);
                        clearConsole();
                    }
                    else
                    {
                        cout << "Failed Transaction";
                        QThread::sleep(2);
                        clearConsole();
                    }


                    break;
                case '6':
                    cout << "Exiting...";
                    QThread::sleep(2);
                    exit(0);
                    break;
                default:
                    cout << "Invalid Service ID" << endl;
                    QThread::sleep(2);
                    clearConsole();
                    break;
                }
            }
        }

    }



    return a.exec();
}



