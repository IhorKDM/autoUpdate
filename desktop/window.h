#pragma once
#include <QNetworkAccessManager>
#include <QUrl>
#include <QWidget>
class QLabel;
class QPushButton;

class Window : public QWidget {
public:
    explicit Window(QString repository = RELEASE_REPOSITORY,
                    QUrl apiUrl = {}, bool checkOnStart = true);
    void checkForUpdates();
private:
    QString repository_;
    QUrl apiUrl_;
    QNetworkAccessManager network_;
    QLabel *status_;
    QPushButton *update_;
    QPushButton *check_;
    QUrl downloadUrl_;
};
