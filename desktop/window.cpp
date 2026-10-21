#include "window.h"
#include "release.h"
#include <QDesktopServices>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

Window::Window(QString repository, QUrl apiUrl, bool checkOnStart)
    : repository_(std::move(repository)), apiUrl_(std::move(apiUrl)) {
    setWindowTitle("Text Echo " APP_VERSION);
    setMinimumWidth(420);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(14);
    auto *title = new QLabel("Text Echo");
    title->setStyleSheet("font-size: 26px; font-weight: bold");
    layout->addWidget(title);
    layout->addWidget(new QLabel("Type two messages below. Press Enter to show them."));
    auto *label = new QLabel("First text:");
    layout->addWidget(label);
    auto *entry = new QLineEdit;
    entry->setObjectName("entry");
    entry->setPlaceholderText("Hello from Qt 6");
    label->setBuddy(entry);
    layout->addWidget(entry);
    auto *secondLabel = new QLabel("Second text:");
    layout->addWidget(secondLabel);
    auto *secondEntry = new QLineEdit;
    secondEntry->setObjectName("secondEntry");
    secondEntry->setPlaceholderText("Another message");
    secondLabel->setBuddy(secondEntry);
    layout->addWidget(secondEntry);
    auto *show = new QPushButton("Show text");
    show->setObjectName("show");
    layout->addWidget(show);
    auto *output = new QLabel("Your text will appear here.");
    output->setObjectName("output");
    output->setTextFormat(Qt::PlainText);
    output->setWordWrap(true);
    output->setMinimumHeight(60);
    output->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(new QLabel("First output:"));
    layout->addWidget(output);
    auto *secondOutput = new QLabel("Your second text will appear here.");
    secondOutput->setObjectName("secondOutput");
    secondOutput->setTextFormat(Qt::PlainText);
    secondOutput->setWordWrap(true);
    secondOutput->setMinimumHeight(60);
    secondOutput->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(new QLabel("Second output:"));
    layout->addWidget(secondOutput);
    connect(show, &QPushButton::clicked, this, [entry, output, secondEntry, secondOutput] {
        output->setText(entry->text());
        secondOutput->setText(secondEntry->text());
    });
    connect(entry, &QLineEdit::returnPressed, show, &QPushButton::click);
    connect(secondEntry, &QLineEdit::returnPressed, show, &QPushButton::click);
    status_ = new QLabel;
    status_->setObjectName("updateStatus");
    status_->setWordWrap(true);
    layout->addWidget(status_);
    update_ = new QPushButton("Update — download new version");
    update_->setObjectName("update");
    update_->hide();
    layout->addWidget(update_);
    connect(update_, &QPushButton::clicked, this, [this] {
        if (!QDesktopServices::openUrl(downloadUrl_))
            status_->setText("Could not open your browser. Try the release page on GitHub.");
    });
    check_ = new QPushButton("Check for updates");
    check_->setObjectName("checkUpdates");
    layout->addWidget(check_);
    connect(check_, &QPushButton::clicked, this, &Window::checkForUpdates);
    auto *version = new QLabel("Version " APP_VERSION " · macOS / Windows");
    layout->addWidget(version);
    if (checkOnStart) QTimer::singleShot(0, this, &Window::checkForUpdates);
    resize(480, 610);
}

void Window::checkForUpdates() {
    update_->hide();
    if (repository_.isEmpty()) {
        status_->setText("Update checks are not configured for this build.");
        return;
    }
    check_->setEnabled(false);
    status_->setText("Checking for updates…");
    const QUrl endpoint = apiUrl_.isEmpty()
        ? QUrl("https://api.github.com/repos/" + repository_ + "/releases/latest") : apiUrl_;
    QNetworkRequest request(endpoint);
    request.setRawHeader("User-Agent", "TextEcho/" APP_VERSION);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setTransferTimeout(10000);
    auto *reply = network_.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        check_->setEnabled(true);
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 404) {
            status_->setText("No published releases yet."); return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            status_->setText("Could not check for updates. Check your connection and try again."); return;
        }
        const auto bytes = reply->readAll();
        if (!QJsonDocument::fromJson(bytes).isObject()) {
            status_->setText("Could not read release information. Try again later."); return;
        }
        const auto release = parseRelease(bytes, repository_, APP_VERSION, platformAssetName());
        if (release.downloadUrl.isEmpty()) {
            status_->setText("No newer compatible release is available."); return;
        }
        downloadUrl_ = release.downloadUrl;
        status_->setText("New version available: " + release.version
                         + ". Download it, quit this app, and replace the old copy.");
        update_->show();
    });
}
