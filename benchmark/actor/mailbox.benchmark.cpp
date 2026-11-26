#include <benchmark/benchmark.h>

#include <mln/actor/actor.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/util/thread.hpp>

#include <cstddef>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t messagesPerSender = 10000;

class Counter {
public:
    Counter(mln::ActorRef<Counter>) {}

    void increment() { ++count; }
    std::size_t get() const { return count; }

private:
    std::size_t count = 0;
};

// Sends `messagesPerSender` messages from each of `senders` threads, then waits until all of them are received.
void sendMessages(mln::ActorRef<Counter> counter, std::size_t senders) {
    std::vector<std::thread> threads;
    threads.reserve(senders);
    for (std::size_t i = 0; i < senders; ++i) {
        threads.emplace_back([counter] {
            for (std::size_t j = 0; j < messagesPerSender; ++j) {
                counter.invoke(&Counter::increment);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    benchmark::DoNotOptimize(counter.ask(&Counter::get).get());
}

void Mailbox_ThreadPool(benchmark::State& state) {
    const auto senders = static_cast<std::size_t>(state.range(0));
    mln::Actor<Counter> counter(mln::Scheduler::GetBackground());

    for (auto _ : state) {
        sendMessages(counter.self(), senders);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * senders * messagesPerSender));
}

void Mailbox_RunLoop(benchmark::State& state) {
    const auto senders = static_cast<std::size_t>(state.range(0));
    mln::util::Thread<Counter> counter("Counter");

    for (auto _ : state) {
        sendMessages(counter.actor(), senders);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * senders * messagesPerSender));
}

} // namespace

BENCHMARK(Mailbox_ThreadPool)->Arg(1)->Arg(4)->UseRealTime();
BENCHMARK(Mailbox_RunLoop)->Arg(1)->Arg(4)->UseRealTime();
