#pragma once

namespace mbgl {

/// Run a lambda on scope exit
template <typename Func>
struct Scoped {
    Scoped(Func&& fn)
        : cb(std::move(fn)){};
    ~Scoped() { cb(); }

    Scoped(const Scoped&) = delete;
    Scoped(Scoped&&) noexcept = delete;
    Scoped& operator=(const Scoped&) = delete;
    Scoped& operator=(Scoped&&) noexcept = delete;

private:
    Func cb;
};

} // namespace mbgl
