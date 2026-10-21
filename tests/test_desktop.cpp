#include "release.h"
#include "window.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

static QByteArray releaseJson(QString tag = "v0.1.1", QString url = {}) {
    const auto asset = platformAssetName();
    if (url.isEmpty()) url = "https://github.com/learner/demo/releases/download/" + tag + "/" + asset;
    return QJsonDocument(QJsonObject{
        {"tag_name", tag}, {"draft", false}, {"prerelease", false},
        {"assets", QJsonArray{QJsonObject{{"name", asset}, {"browser_download_url", url}}}}
    }).toJson(QJsonDocument::Compact);
}

class DesktopTests : public QObject {
    Q_OBJECT
private slots:
    void echoesLiteralText() {
        Window window({}, {}, false);
        auto *entry = window.findChild<QLineEdit *>("entry");
        auto *output = window.findChild<QLabel *>("output");
        entry->setText("<b>Hello</b> & Qt");
        window.findChild<QPushButton *>("show")->click();
        QCOMPARE(output->text(), entry->text());
        QCOMPARE(output->textFormat(), Qt::PlainText);
        entry->setText("Second message");
        QTest::keyClick(entry, Qt::Key_Return);
        QCOMPARE(output->text(), QString("Second message"));
        entry->clear();
        window.findChild<QPushButton *>("show")->click();
        QVERIFY(output->text().isEmpty());
    }
    void showsBothFieldsIndependently() {
        Window window({}, {}, false);
        auto *first = window.findChild<QLineEdit *>("entry");
        auto *second = window.findChild<QLineEdit *>("secondEntry");
        auto *firstOutput = window.findChild<QLabel *>("output");
        auto *secondOutput = window.findChild<QLabel *>("secondOutput");
        QVERIFY(second);
        QVERIFY(secondOutput);
        first->setText("First message");
        second->setText("<b>Second message</b> & Qt");
        window.findChild<QPushButton *>("show")->click();
        QCOMPARE(firstOutput->text(), first->text());
        QCOMPARE(secondOutput->text(), second->text());
        QCOMPARE(secondOutput->textFormat(), Qt::PlainText);
        second->setText("Changed through Enter");
        QTest::keyClick(second, Qt::Key_Return);
        QCOMPARE(secondOutput->text(), second->text());
        QCOMPARE(firstOutput->text(), QString("First message"));
        second->clear();
        QTest::keyClick(first, Qt::Key_Return);
        QVERIFY(secondOutput->text().isEmpty());
        QCOMPARE(firstOutput->text(), QString("First message"));
    }
    void numericVersionComparison() {
        QVERIFY(!parseRelease(releaseJson("v0.1.10"), "learner/demo", "0.1.9", platformAssetName()).downloadUrl.isEmpty());
        QVERIFY(parseRelease(releaseJson(), "learner/demo", "0.1.1", platformAssetName()).downloadUrl.isEmpty());
        QVERIFY(parseRelease(releaseJson(), "learner/demo", "1.0.0", platformAssetName()).downloadUrl.isEmpty());
    }
    void ignoresInvalidAndUnsafeReleases() {
        for (const QString &tag : {"v0.2.0-beta", "garbage", "v01.2.0", "v0.2"})
            QVERIFY(parseRelease(releaseJson(tag), "learner/demo", "0.1.0", platformAssetName()).downloadUrl.isEmpty());
        QVERIFY(parseRelease("not json", "learner/demo", "0.1.0", platformAssetName()).downloadUrl.isEmpty());
        QVERIFY(parseRelease(releaseJson("v0.1.1", "https://example.com/evil.zip"), "learner/demo", "0.1.0", platformAssetName()).downloadUrl.isEmpty());
        QVERIFY(parseRelease(releaseJson(), "learner/demo", "0.1.0", "missing.zip").downloadUrl.isEmpty());
        for (const auto *field : {"draft", "prerelease"}) {
            auto object = QJsonDocument::fromJson(releaseJson()).object();
            object[field] = true;
            QVERIFY(parseRelease(QJsonDocument(object).toJson(), "learner/demo", "0.1.0", platformAssetName()).downloadUrl.isEmpty());
        }
    }
    void startupChecksOverHttp() {
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        const auto body = releaseJson("v99.0.0");
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            auto *socket = server.nextPendingConnection();
            connect(socket, &QTcpSocket::readyRead, socket, [socket, body] {
                socket->readAll();
                socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                              + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
                socket->disconnectFromHost();
            });
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        });
        Window window("learner/demo", QUrl("http://127.0.0.1:" + QString::number(server.serverPort())), true);
        auto *status = window.findChild<QLabel *>("updateStatus");
        QTRY_VERIFY_WITH_TIMEOUT(status->text().contains("New version available: 99.0.0"), 5000);
        QVERIFY(!window.findChild<QPushButton *>("update")->isHidden());
        QVERIFY(window.findChild<QPushButton *>("checkUpdates")->isEnabled());
    }
};
QTEST_MAIN(DesktopTests)
#include "test_desktop.moc"
