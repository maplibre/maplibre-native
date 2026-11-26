#pragma once

#include <mln/storage/http_file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/util/async_request.hpp>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace mln {

class Response;

class HTTPRequest : public AsyncRequest {
public:
    HTTPRequest(HTTPFileSource::Impl*, const Resource&, FileSource::Callback);
    virtual ~HTTPRequest();

    QUrl requestUrl() const;
    QNetworkRequest networkRequest() const;

    void handleNetworkReply(QNetworkReply*, const QByteArray& data);

private:
    HTTPFileSource::Impl* m_context;
    Resource m_resource;
    FileSource::Callback m_callback;

    bool m_handled = false;
};

} // namespace mln
