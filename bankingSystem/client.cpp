#include "client.h"
#include "qurl.h"
#include <QTcpSocket>
#include <QThread>

// Create a TCP socket
static QTcpSocket socket;

// Connect to a server
static QString serverAddress = "tcp://0.tcp.eu.ngrok.io:15120"; // Replace with your server's IP address



Client:: Client()
{

    QUrl url(serverAddress);

    if (url.scheme() == "tcp" && url.isValid()) {
        // Extract the host and port from the URL
        QString host = url.host();
        quint16 port = static_cast<quint16>(url.port(0)); // Use port 0 to get the default if not specified

        // Connect to the host
        socket.connectToHost(host, port);
    } else {
        qDebug() << "Invalid server address format";
    }
   // socket.connectToHost(QHostAddress(serverAddress), port);

    // Check if the connection is successful
    if (socket.waitForConnected())
    {
        connectedToServer = true;

        // Send data to the server
        QString message = "CC";
        socket.write(message.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            qDebug() << "Received response from server:" << socket.readAll();
        } else
        {
            qDebug() << "Timeout waiting for response.";
        }

    } else
    {
        connectedToServer = false;
        qDebug() << "Failed to connect to server.";
        qDebug() << "Exiting...";
        QThread::sleep(2);
        exit(1);

    }
}


//Admin & Client Functions
bool Client::login(QString &m_username, QString &m_password)
{
    if(connectedToServer)
    {
        QString credentials = "L:" + m_username +','+ m_password;
        QByteArray response;

        socket.write(credentials.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "Valid Credentials")
            {
                return true;
            }
            else
            {
                return false;
            }

            //qDebug() << "Received response from server:" << response;
        } else
        {
            qDebug() << "Timeout waiting for response.";
            return false;
        }
    }
    else
    {
        return false;
    }
}

int Client::viewAccountBalance(QString m_accountNumber)
{
    if(connectedToServer)
    {
        QString data = "B:" + m_accountNumber;
        QByteArray response;

        socket.write(data.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "none")
            {
                return 0;
            }
            else
            {
                return response.toInt();
            }

        } else
        {
            qDebug() << "Timeout waiting for response.";
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

QString Client::viewTransactionHistory(QString m_accountNumber, int count)
{
    if(connectedToServer)
    {
        QString data = "H:" + m_accountNumber + ',' + QString::number(count);
        QByteArray response;

        socket.write(data.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "none")
            {
                return "none";
            }
            else
            {
                return QString::fromUtf8(response);
            }

        } else
        {
            qDebug() << "Timeout waiting for response.";
            return 0;
        }
    }
    else
    {
        return 0;
    }
}


//Client Specific Functions
QString Client::getAccountNumber(QString m_username)
{
    if(connectedToServer)
    {
        QString data = "G:" + m_username;
        QByteArray response;

        socket.write(data.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "none")
            {
                return "NaN";
            }
            else
            {
                return QString::fromUtf8(response);
            }

            //qDebug() << "Received response from server:" << response;
        } else
        {
            qDebug() << "Timeout waiting for response.";
            return "NaN";
        }
    }
    else
    {
        return "NaN";
    }
}

bool Client::makeTransaction(QString m_accountNumber, signed int transactionAmount)
{
    if(connectedToServer)
    {
        QString data = "T:" + m_accountNumber +',' + QString::number(transactionAmount);
        QByteArray response;

        socket.write(data.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "success")
            {
                return true;
            }
            else
            {
                return false;
            }

            //qDebug() << "Received response from server:" << response;
        } else
        {
            qDebug() << "Timeout waiting for response.";
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Client::transferAmount(QString fromAccountNumber, QString toAccountNumber, unsigned int transactionAmount)
{
    if(connectedToServer)
    {
        QString data = "W:" + fromAccountNumber +',' + toAccountNumber + ',' + QString::number(transactionAmount);
        QByteArray response;

        socket.write(data.toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "success")
            {
                return true;
            }
            else
            {
                return false;
            }

            //qDebug() << "Received response from server:" << response;
        } else
        {
            qDebug() << "Timeout waiting for response.";
            return false;
        }
    }
    else
    {
        return false;
    }
}
