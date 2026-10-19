#pragma once

#include <mln/storage/file_source.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/client_options.hpp>
#include <mln/util/thread.hpp>

namespace mln {
// File source for supporting .pmtiles maps
class PMTilesFileSource : public FileSource {
public:
    PMTilesFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions);
    ~PMTilesFileSource() override;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource&) const override;

    void setResourceOptions(ResourceOptions) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions) override;
    ClientOptions getClientOptions() override;

private:
    class Impl;
    std::unique_ptr<util::Thread<Impl>> thread; // impl
};

} // namespace mln
