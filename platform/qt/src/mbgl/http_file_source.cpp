#include "http_file_source.hpp"
#include "http_request.hpp"

#include <mbgl/util/logging.hpp>

#include <QByteArray>
#include <QDir>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QSslConfiguration>

namespace mbgl {

HTTPFileSource::Impl::Impl(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : m_manager(new QNetworkAccessManager(this)),
      m_resourceOptions(resourceOptions.clone()),
      m_clientOptions(clientOptions.clone()) {
    QNetworkProxyFactory::setUseSystemConfiguration(true);
}

void HTTPFileSource::Impl::request(HTTPRequest* req) {
    QUrl url = req->requestUrl();

    QPair<QPointer<QNetworkReply>, QVector<HTTPRequest*>>& data = m_pending[url];
    QVector<HTTPRequest*>& requestsVector = data.second;
    requestsVector.append(req);

    if (requestsVector.size() > 1) {
        return;
    }

    QNetworkRequest networkRequest = req->networkRequest();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    networkRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
#endif

    data.first = m_manager->get(networkRequest);
    connect(data.first, &QNetworkReply::finished, this, &HTTPFileSource::Impl::onReplyFinished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(data.first, &QNetworkReply::errorOccurred, this, &HTTPFileSource::Impl::onReplyFinished);
#else
    connect(data.first, &QNetworkReply::error, this, &HTTPFileSource::Impl::onReplyFinished);
#endif
}

void HTTPFileSource::Impl::cancel(HTTPRequest* req) {
    QUrl url = req->requestUrl();

    auto it = m_pending.find(url);
    if (it == m_pending.end()) {
        return;
    }

    QPair<QPointer<QNetworkReply>, QVector<HTTPRequest*>>& data = it.value();
    QNetworkReply* reply = data.first;
    QVector<HTTPRequest*>& requestsVector = data.second;

    for (int i = 0; i < requestsVector.size(); ++i) {
        if (req == requestsVector.at(i)) {
            requestsVector.remove(i);
            break;
        }
    }

    if (requestsVector.empty()) {
        m_pending.erase(it);
        if (reply) reply->abort();
    }
}

void HTTPFileSource::Impl::onReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    const QUrl& url = reply->request().url();

    auto it = m_pending.find(url);
    if (it == m_pending.end()) {
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QVector<HTTPRequest*>& requestsVector = it.value().second;

    // Cannot use the iterator to walk the requestsVector
    // because calling handleNetworkReply() might get
    // requests added to the requestsVector.
    while (!requestsVector.isEmpty()) {
        requestsVector.takeFirst()->handleNetworkReply(reply, data);
    }

    m_pending.erase(it);
    reply->deleteLater();
}

void HTTPFileSource::Impl::setResourceOptions(ResourceOptions options) {
    m_resourceOptions = options;
}

ResourceOptions HTTPFileSource::Impl::getResourceOptions() {
    return m_resourceOptions.clone();
}

void HTTPFileSource::Impl::setClientOptions(ClientOptions options) {
    m_clientOptions = options;
}

ClientOptions HTTPFileSource::Impl::getClientOptions() {
    return m_clientOptions.clone();
}

HTTPFileSource::HTTPFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(resourceOptions, clientOptions)) {}

HTTPFileSource::~HTTPFileSource() = default;

std::unique_ptr<AsyncRequest> HTTPFileSource::request(const Resource& resource, Callback callback) {
    return std::make_unique<HTTPRequest>(impl.get(), resource, callback);
}

void HTTPFileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(options.clone());
}

ResourceOptions HTTPFileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

void HTTPFileSource::setClientOptions(ClientOptions options) {
    impl->setClientOptions(options.clone());
}

ClientOptions HTTPFileSource::getClientOptions() {
    return impl->getClientOptions();
}

} // namespace mbgl
