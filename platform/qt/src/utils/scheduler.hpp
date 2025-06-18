#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/util.hpp>

#include <QObject>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace QMapLibre {

class Scheduler : public QObject, public mbgl::Scheduler {
    Q_OBJECT

public:
    Scheduler();
    ~Scheduler() override;

    // mbgl::Scheduler implementation.
    void schedule(const mbgl::util::SimpleIdentity, std::function<void()>&& function) final;
    void schedule(std::function<void()>&& function) final;

    void waitForEmpty(const mbgl::util::SimpleIdentity tag = mbgl::util::SimpleIdentity::Empty) override;

    mapbox::base::WeakPtr<mbgl::Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    void processEvents();

signals:
    void needsProcessing();

private:
    MBGL_STORE_THREAD(tid);

    std::mutex m_taskQueueMutex;
    std::condition_variable cvEmpty;
    std::atomic<std::size_t> pendingItems;
    std::queue<std::function<void()>> m_taskQueue;
    mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

} // namespace QMapLibre
