#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>
#include <QSqlError>


QMutex printLock;

class ServerThread : public QThread
{
public:
    ServerThread(qintptr socketDescriptor)
        : socketDescriptor(socketDescriptor)
    {
    }

protected:
    void run() override
    {
        // Connect to the SQLite database (creates a new file if it doesn't exist)
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("example.db");

        if (!db.open())
        {
            qDebug() << "Error: Unable to open database";
            return;
        }

        // Create a table
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS admins ("
                   "id INTEGER PRIMARY KEY,"
                   "username TEXT NOT NULL,"
                   "password TEXT NOT NULL)");

        query.exec("CREATE TABLE IF NOT EXISTS clients ("
                   "id INTEGER PRIMARY KEY,"
                   "username TEXT NOT NULL,"
                   "password TEXT NOT NULL,"
                   "accountNumber TEXT NOT NULL,"
                   "fullName TEXT NOT NULL,"
                   "balance INTEGER NOT NULL,"
                   "age INTEGER NOT NULL)");

        query.exec("CREATE TABLE IF NOT EXISTS transactions ("
                   "id INTEGER PRIMARY KEY,"
                   "accountNumber TEXT NOT NULL,"
                   "amount INTEGER NOT NULL,"
                   "date DATE NOT NULL)");

        // Query the data
        query.exec("SELECT * FROM clients");
        while (query.next())
        {
            qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toString()
                     << query.value(3).toString() << query.value(4).toString() << query.value(5).toInt()
                     << query.value(6).toInt();
        }

        query.exec("SELECT * FROM admins");
        while (query.next())
        {
            qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toString();
        }

        query.exec("SELECT * FROM transactions");
        while (query.next())
        {
            qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toInt() << query.value(3).toString();
        }

        // Create a new socket for communication
        QTcpSocket socket;
        if (!socket.setSocketDescriptor(socketDescriptor))
        {
            qDebug() << "Error: Unable to set socket descriptor";
            return;
        }

        int user =  0;

        while (socket.isValid())
        {
            // data received from client
            if (socket.waitForReadyRead(-1))
            {
                QByteArray data = socket.readAll();
                data = data.trimmed();  // remove leading/trailing whitespace

                if (data.isEmpty())
                {
                    continue;
                }

                if (data.startsWith('L')) // LOGIN DONE
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    if(user == 1)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM admins WHERE username = ? AND password = ?");
                        query.bindValue(0, dataCSV[0]);
                        query.bindValue(1, dataCSV[1]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Username and password match in the database";
                                socket.write("Valid Credentials");
                            } else
                            {
                                qDebug() << "Username or password do not match in the database";
                                socket.write("Wrong Credentials");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }
                    else if(user == 2)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE username = ? AND password = ?");
                        query.bindValue(0, dataCSV[0]);
                        query.bindValue(1, dataCSV[1]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Username and password match in the database";
                                socket.write("Valid Credentials");
                            } else
                            {
                                qDebug() << "Username or password do not match in the database";
                                socket.write("Wrong Credentials");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }

                }

                else if (data.startsWith('W')) //TRANSFER AMOUNT
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    int check = 0 , flag = 0 ;

                    QSqlQuery query;
                    query.prepare("SELECT * FROM clients WHERE accountNumber = :accountNumber");
                    query.bindValue(":accountNumber",dataCSV[0]);

                    if (query.exec() && query.next())
                    {
                        qDebug() << "First Account number Exist";
                        check ++;
                    }
                    else
                    {
                        qDebug() << "Error checking account one existence";
                        socket.write("fail");
                    }


                    query.prepare("SELECT * FROM clients WHERE accountNumber = :accountNumber");
                    query.bindValue(":accountNumber",dataCSV[1] );

                    if (query.exec() && query.next())
                    {
                        qDebug() << "Second Account number Exist";
                        check ++;
                    }
                    else
                    {
                        qDebug() << "Error checking account Two existence";
                        socket.write("fail");
                    }


                    if(check == 2)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE accountNumber = ?");
                        query.bindValue(0, dataCSV[0]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Account Number was found in the database";

                                if(query.value(5).toInt() >= dataCSV[2].toInt())
                                {
                                    flag = 1;
                                    int newBalance = query.value(5).toInt() - dataCSV[2].toInt() ;
                                    query.prepare("UPDATE clients SET balance = :newBalance WHERE accountNumber = :accountNumber");
                                    query.bindValue(":newBalance", newBalance);
                                    query.bindValue(":accountNumber", dataCSV[0]);
                                    if (query.exec())
                                    {
                                        qDebug() << "Update successful";

                                    } else
                                    {
                                        qDebug() << "Error executing update query";

                                    }
                                }
                                else
                                {
                                    socket.write("fail");
                                }
                            }
                        }
                        else
                        {
                            socket.write("fail");
                        }

                        query.prepare("SELECT * FROM clients WHERE accountNumber = ?");
                        query.bindValue(0, dataCSV[1]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next() && flag == 1)
                            {
                                qDebug() << "Account Number was found in the database";

                                int newBalance = query.value(5).toInt() + dataCSV[2].toInt() ;
                                query.prepare("UPDATE clients SET balance = :newBalance WHERE accountNumber = :accountNumber");
                                query.bindValue(":newBalance", newBalance);
                                query.bindValue(":accountNumber", dataCSV[1]);
                                if (query.exec())
                                {
                                    qDebug() << "Update successful";
                                    socket.write("success");
                                } else
                                {
                                    qDebug() << "Error executing update query";
                                    socket.write("fail");
                                }
                            }
                        }
                        else
                        {
                            socket.write("fail");
                        }
                    }
                }

                else if (data.startsWith('H')) //VIEW TRANSACTION HISTORY
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    if(user == 1)
                    {

                        QSqlQuery query;
                        query.prepare("SELECT * FROM transactions WHERE accountNumber = :accountNumber LIMIT :numRows");
                        query.bindValue(":numRows", dataCSV[1].toInt());
                        query.bindValue(":accountNumber", dataCSV[0]);

                        if (query.exec())
                        {
                            while (query.next()) {
                                // Process each row
                                QString rowData =   query.value(1).toString() + ","
                                                  + query.value(2).toString() + ","
                                                  + query.value(3).toString() + "|";

                                // Send the row data over the socket
                                socket.write(rowData.toUtf8());

                            }
                        }
                        else
                        {
                            qDebug() << "Error";
                            socket.write("none");
                        }

                    }
                    else if (user == 2)
                    {

                        QSqlQuery query;
                        query.prepare("SELECT * FROM transactions WHERE accountNumber = :accountNumber LIMIT :numRows");
                        query.bindValue(":numRows", dataCSV[1].toInt());
                        query.bindValue(":accountNumber", dataCSV[0]);

                        if (query.exec())
                        {
                            while (query.next())
                            {
                                // Process each row
                                QString rowData = query.value(1).toString() + ","
                                                  + query.value(2).toString() + ","
                                                  + query.value(3).toString() + "|";

                                // Send the row data over the socket
                                socket.write(rowData.toUtf8());

                            }
                        }
                        else
                        {
                            qDebug() << "Error";
                            socket.write("none");
                        }

                    }
                }


                else if (data.startsWith('Z')) // VIEW BANKDATABASE
                {
                    if(user == 1)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients");

                        if (query.exec())
                        {
                            while (query.next())
                            {
                                // Process each row
                                QString rowData =   query.value(1).toString() + ","
                                                  + query.value(3).toString() + ","
                                                  + query.value(4).toString() + ","
                                                  + query.value(5).toString() + ","
                                                  + query.value(6).toString() + "|";

                                // Send the row data over the socket
                                socket.write(rowData.toUtf8());

                            }
                        }
                        else
                        {
                            qDebug() << "Error";
                            socket.write("none");
                        }

                    }
                    else
                    {
                        //do nothing
                    }
                }


                else if (data.startsWith('G')) //GET ACCOUNT NUMBER [DONE]
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    if(user == 1)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE username = ?");
                        query.bindValue(0, dataCSV[0]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Account Number was found in the database";
                                socket.write(query.value(3).toString().toUtf8());
                            } else
                            {
                                qDebug() << "Account Number was not found in the database";
                                socket.write("none");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }

                    else if(user == 2)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE username = ?");
                        query.bindValue(0, dataCSV[0]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Account Number was found in the database";
                                socket.write(query.value(3).toString().toUtf8());
                            } else //THIS CONDITION IS IMPOSSIBLE
                            {
                                qDebug() << "Account Number was not found in the database";
                                socket.write("none");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }
                }


                else if (data.startsWith('D')) // ADMIN DELETE USER
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    QString accountNumberToDelete = dataCSV[0];

                    QSqlQuery query;
                    query.prepare("DELETE FROM clients WHERE accountNumber = :accountNumber");
                    query.bindValue(":accountNumber", accountNumberToDelete);

                    if (query.exec()) {

                        if (query.exec())
                        {

                            if (query.next())
                            {
                                qDebug() << "Deletion successful";
                                socket.write("success");
                            } else
                            {
                                qDebug() << "Error executing delete query";
                                socket.write("fail");
                            }
                        }
                    }
                }

                else if (data.startsWith('U')) // ADMIN UPDATE USER
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    QSqlQuery query;
                    query.prepare("UPDATE clients SET username = :newUsername, password = :newPassword, fullName = :newFullName, balance = :newBalance, age = :newAge WHERE accountNumber = :accountNumber");
                    query.bindValue(":newUsername", dataCSV[1]);
                    query.bindValue(":newPassword", dataCSV[2]);
                    query.bindValue(":newFullName", dataCSV[3]);
                    query.bindValue(":newBalance", dataCSV[4].toInt());
                    query.bindValue(":newAge", dataCSV[5].toInt());
                    query.bindValue(":accountNumber", dataCSV[0]);

                    if (query.exec())
                    {
                        qDebug() << "Update successful";
                        socket.write("success");
                    }
                    else
                    {
                        qDebug() << "Error executing update query" ;
                        socket.write("fail");
                    }
                }


                else if (data.startsWith('T')) //MAKE TRANSACATION
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    int success_flag = 0;

                    QSqlQuery query;
                    query.prepare("SELECT * FROM clients WHERE accountNumber = ?");
                    query.bindValue(0, dataCSV[0]);

                    if (query.exec())
                    {
                        // Check if any rows were returned
                        if (query.next())
                        {
                            qDebug() << "Account Number was found in the database";
                            if (dataCSV[1].toInt() > 0)
                            {
                                if(dataCSV[1].toInt() != 0)
                                {
                                    int newBalance = query.value(5).toInt() + dataCSV[1].toInt() ;
                                    query.prepare("UPDATE clients SET balance = :newBalance WHERE accountNumber = :accountNumber");
                                    query.bindValue(":newBalance", newBalance);
                                    query.bindValue(":accountNumber", dataCSV[0]);
                                    if (query.exec())
                                    {
                                        qDebug() << "Update successful";
                                        success_flag = 1;
                                        socket.write("success");
                                    }
                                    else
                                    {
                                        qDebug() << "Error executing update query";
                                        socket.write("fail");
                                    }
                                }
                                else
                                {
                                    socket.write("fail");
                                }
                            }
                            else if(dataCSV[1].toInt() < 0)
                            {
                                if((query.value(5).toInt()) >= abs(dataCSV[1].toInt()) && (dataCSV[1].toInt() != 0))
                                {
                                    int newBalance = query.value(5).toInt() + dataCSV[1].toInt() ;
                                    query.prepare("UPDATE clients SET balance = :newBalance WHERE accountNumber = :accountNumber");
                                    query.bindValue(":newBalance", newBalance);
                                    query.bindValue(":accountNumber", dataCSV[0]);
                                    if (query.exec())
                                    {
                                        qDebug() << "Update successful";
                                        success_flag = 1;
                                        socket.write("success");
                                    }
                                    else
                                    {
                                        qDebug() << "Error executing update query";
                                    }
                                }
                                else
                                {
                                    socket.write("fail");
                                }
                            }


                        }
                        else //THIS CONDITION IS IMPOSSIBLE
                        {
                            qDebug() << "Account Number was not found in the database";
                            socket.write("fail");
                        }
                    }
                    else
                    {
                        qDebug() << "Error executing query";
                        socket.write("fail");
                    }

                    if(success_flag == 1)
                    {
                        QSqlQuery query1 ;
                        query1.prepare("INSERT INTO transactions (accountNumber, amount, date) VALUES (?, ?, ?)");
                        query1.bindValue(0, dataCSV[0]);  // Assuming dataCSV[0] is the accountNumber
                        query1.bindValue(1, dataCSV[1].toInt());  // Assuming dataCSV[1] is the amount

                        // Create a QDateTime object with the desired date
                        QDateTime myDate = QDateTime::currentDateTime();  // Replace this with your specific date
                        query1.bindValue(2, myDate.toString(Qt::ISODate));

                        qDebug() << dataCSV[0] <<"           "<<dataCSV[1].toInt()<<"         "<<myDate.toString(Qt::ISODate);

                        if (query1.exec())
                        {
                            // Query executed successfully
                        }
                        else
                        {
                            // Handle the error
                            qDebug() << "Error ";
                        }
                    }
                    else
                    {
                        //do nothing
                    }

                }

                else if (data.startsWith('B')) //GET ACCOUNT BALANCE [DONE]
                {
                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    if(user == 1)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE accountNumber = ?");
                        query.bindValue(0, dataCSV[0]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Account Balance was found in the database";
                                socket.write(query.value(5).toString().toUtf8());
                            }
                            else
                            {
                                qDebug() << "Account Balance was not found in the database";
                                socket.write("none");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }

                    else if(user == 2)
                    {
                        QSqlQuery query;
                        query.prepare("SELECT * FROM clients WHERE accountNumber = ?");
                        query.bindValue(0, dataCSV[0]);

                        if (query.exec())
                        {
                            // Check if any rows were returned
                            if (query.next())
                            {
                                qDebug() << "Account Balance was found in the database";
                                socket.write(query.value(5).toString().toUtf8());
                            }
                            else //THIS CONDITION IS IMPOSSIBLE
                            {
                                qDebug() << "Account Balance was not found in the database";
                                socket.write("none");
                            }
                        }
                        else
                        {
                            qDebug() << "Error executing query";
                        }
                    }
                }

                else if (data.startsWith('A')) //ADD USER [DONE]
                {

                    QStringList dataCSV = QString(data.mid(2)).split(',');
                    qDebug() << dataCSV;

                    if(dataCSV[0].toInt() == 1)
                    {
                        qDebug() << "Create New Admin with Data: " << data.mid(2);

                        query.prepare("INSERT INTO admins (username, password) "
                                      "VALUES (?, ?)");
                        query.bindValue(0, dataCSV[1]);
                        query.bindValue(1, dataCSV[2]);
                        query.exec();




                        query.exec("SELECT * FROM admins");
                        while (query.next())
                        {
                            qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toString();
                        }

                        socket.write("Valid Data");

                    }
                    else if (dataCSV[0].toInt() == 2)
                    {
                        qDebug() << "Create New Client with Data: " << data.mid(2);

                        // Check if accountNumber already exists
                        QSqlQuery checkQuery;
                        checkQuery.prepare("SELECT COUNT(*) FROM clients WHERE accountNumber = ?");
                        checkQuery.bindValue(0, dataCSV[3]);  // Assuming dataCSV[3] contains the accountNumber
                        checkQuery.exec();

                        if (checkQuery.next() && checkQuery.value(0).toInt() == 0)
                        {
                            // AccountNumber doesn't exist, perform the insertion
                            QSqlQuery query;
                            query.prepare("INSERT INTO clients (username, password, accountNumber, fullName, balance, age) "
                                          "VALUES (?, ?, ?, ?, ?, ?)");
                            query.bindValue(0, dataCSV[1]);
                            query.bindValue(1, dataCSV[2]);
                            query.bindValue(2, dataCSV[3]);
                            query.bindValue(3, dataCSV[4]);
                            query.bindValue(4, dataCSV[5].toInt());
                            query.bindValue(5, dataCSV[6].toInt());

                            if (query.exec())
                            {
                                qDebug() << "Insertion successful!";

                                // Query the data
                                query.exec("SELECT * FROM clients");
                                while (query.next())
                                {
                                    qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toString()
                                             << query.value(3).toString() << query.value(4).toString() << query.value(5).toInt()
                                             << query.value(6).toInt();
                                }

                                socket.write("Valid Data");
                            }
                            else
                            {
                                qDebug() << "Error during insertion:" << query.lastError().text();
                            }
                        }
                        else
                        {
                            qDebug() << "AccountNumber already exists!";
                            socket.write("none");
                        }
                    }

                }
                else if (user == 0 && QString::fromUtf8(data) == "CA") //CONNECT TO ADMIN [DONE]
                {
                    user = 1 ;
                    socket.write("Connected To Server as Admin");
                }
                else if (user == 0 && QString::fromUtf8(data) == "CC") //CONNECT TO CLIENT [DONE]
                {
                    user = 2 ;
                    socket.write("Connected To Server as Client");
                }
            }
        }
    }

private:
    qintptr socketDescriptor;
};

class Server : public QTcpServer
{
public:
    Server(QObject* parent = nullptr)
        : QTcpServer(parent)
    {
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override
    {
        QThread* thread = new ServerThread(socketDescriptor);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    if (!server.listen(QHostAddress::Any, 12345)) {
        qDebug() << "Error: Unable to start server";
        return 1;
    }

    qDebug() << "Server is listening on port 12345";

    return a.exec();
}
