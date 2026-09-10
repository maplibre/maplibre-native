#include <mln/storage/file_source_request.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/filesystem.hpp>
#include <mln/util/io.hpp>

#if defined(_WIN32)
#include <filesystem>
#include <system_error>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace mln {

void requestLocalFile(const std::string& path,
                      const ActorRef<FileSourceRequest>& req,
                      const std::optional<std::pair<uint64_t, uint64_t>>& dataRange) {
    Response response;
#if defined(_WIN32)
    std::error_code error;
    std::filesystem::file_status status;
    try {
        status = std::filesystem::status(util::pathFromUTF8(path), error);
    } catch (const std::filesystem::filesystem_error& exception) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other, exception.what());
        req.invoke(&FileSourceRequest::setResponse, response);
        return;
    }
    const bool notFound = status.type() == std::filesystem::file_type::not_found;
    const bool isDirectory = std::filesystem::is_directory(status);
#else
    struct stat buf;
    const int result = stat(path.c_str(), &buf);
    const bool notFound = result == -1 && errno == ENOENT;
    const bool isDirectory = result == 0 && S_ISDIR(buf.st_mode);
#endif

    if (notFound || isDirectory) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
    } else {
        auto data = util::readFile(path, dataRange);
        if (!data) {
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                               std::string("Cannot read file ") + path);
        } else {
            response.data = std::make_shared<std::string>(std::move(*data));
        }
    }

    req.invoke(&FileSourceRequest::setResponse, response);
}

} // namespace mln
