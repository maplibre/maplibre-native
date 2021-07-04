#pragma once

#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/resource_options.hpp>

#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QUrl>
#include <QVector>

namespace mbgl {

class HTTPRequest;

class HTTPFileSource::Impl : public QObject
{
    Q_OBJECT

public:
    Impl(const ResourceOptions& options);
    virtual ~Impl() = default;

    void request(HTTPRequest *);
    void cancel(HTTPRequest *);

    void setResourceOptions(ResourceOptions options);
    ResourceOptions getResourceOptions();

public slots:
    void onReplyFinished();

private:
    QMap<QUrl, QPair<QPointer<QNetworkReply>, QVector<HTTPRequest *>>> m_pending;
    QNetworkAccessManager *m_manager;
    ResourceOptions m_resourceOptions;
};

} // namespace mbgl
