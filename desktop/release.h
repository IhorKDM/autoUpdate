#pragma once
#include <QByteArray>
#include <QString>
#include <QUrl>

struct Release {
    QString version;
    QUrl downloadUrl;
};
QString platformAssetName();
Release parseRelease(const QByteArray &json, const QString &repository,
                     const QString &currentVersion, const QString &assetName);
