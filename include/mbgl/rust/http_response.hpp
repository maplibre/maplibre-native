#pragma once

#include <mbgl/storage/response.hpp>

#include <memory>
#include <string>
#include <utility>

namespace mbgl {

class HttpResponse {
public:
    explicit HttpResponse(Response* response_)
        : response(response_) {
        if (response) {
            response->data.reset();
        }
    }

    void set_data(std::shared_ptr<const std::string> data_) { response->data = std::move(data_); }

    void set_etag(std::string etag_) { response->etag = std::move(etag_); }

    void set_no_content(bool no_content) { response->noContent = no_content; }

    void set_not_modified(bool not_modified) { response->notModified = not_modified; }

    void set_error(Response::Error::Reason reason, std::string message) {
        response->error = std::make_unique<Response::Error>(reason, std::move(message));
    }

private:
    Response* response;
};

} // namespace mbgl
