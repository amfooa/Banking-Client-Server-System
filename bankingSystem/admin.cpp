#include "admin.h"
#include "qurl.h"
#include <QTcpSocket>
#include <QThread>

// Create a TCP socket
static QTcpSocket socket;

// Connect to a server
static QString serverAddress = "tcp://0.tcp.eu.ngrok.io:15120"; // Replace with your server's IP address

Admin:: Admin()
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
    // Check if the connection is successful

    if (socket.waitForConnected())
    {
        connectedToServer = true;

        // Send data to the server
        QString message = "CA";
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

bool Admin::login(QString &m_username, QString &m_password)
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

        }
         else
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

int Admin::viewAccountBalance(QString m_accountNumber)
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

QString Admin::viewTransactionHistory(QString m_accountNumber, int count)
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

        } 
        else
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

QString Admin::getAccountNumber(QString adminName, QString m_username)
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

        } 
        else
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

QString Admin::viewBankDB(void)
{
    if(connectedToServer)
    {
        QString data = "Z";
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

        } 
        else
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

bool Admin::createUser(QString userData)
{
    if(connectedToServer)
    {

        QByteArray response;

        socket.write(("A:" + userData).toUtf8());
        socket.flush(); // Make sure data is sent

        // Wait for the server's response
        if (socket.waitForReadyRead())
        {
            response = socket.readAll();

            if(QString::fromUtf8(response) == "Valid Data")
            {
                return true;
            }
            else
            {
                return false;
            }

        }
         else
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

bool Admin::deleteUser(QString m_accountNumber)
{
    if(connectedToServer)
    {

        QByteArray response;

        socket.write(("D:" + m_accountNumber).toUtf8());
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

        } 
        else
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

bool Admin::updateUser(QString m_accountNumber, QString newData)
{
    if(connectedToServer)
    {

        QByteArray response;

        socket.write(("U:" + m_accountNumber + ',' + newData).toUtf8());
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

        } 
        else
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
