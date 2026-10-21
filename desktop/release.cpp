#include "release.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QVersionNumber>

QString platformAssetName() {
#if defined(Q_OS_MACOS)
    return "TextEcho-macos-arm64.zip";
#elif defined(Q_OS_WIN)
    return "TextEcho-windows-x64.zip";
#else
    return {};
#endif
}

Release parseRelease(const QByteArray &json, const QString &repository,
                     const QString &currentVersion, const QString &assetName) {
    const auto document = QJsonDocument::fromJson(json);
    if (!document.isObject() || assetName.isEmpty()) return {};
    const auto object = document.object();
    if (object.value("draft").toBool() || object.value("prerelease").toBool()) return {};
    const QString tag = object.value("tag_name").toString();
    static const QRegularExpression stable("^v(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$");
    if (!stable.match(tag).hasMatch()) return {};
    const QString version = tag.mid(1);
    if (QVersionNumber::fromString(version) <= QVersionNumber::fromString(currentVersion)) return {};
    const QUrl expected("https://github.com/" + repository + "/releases/download/" + tag + "/" + assetName);
    for (const auto &value : object.value("assets").toArray()) {
        const auto asset = value.toObject();
        const QUrl url(asset.value("browser_download_url").toString());
        if (asset.value("name").toString() == assetName && url == expected)
            return {version, expected};
    }
    return {};
}
