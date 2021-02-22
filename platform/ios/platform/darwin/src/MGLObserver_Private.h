#import <mbgl/util/observable.hpp>

#import "MGLObserver.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Native C++ peer object

namespace mbgl {
namespace darwin {

class Observer : public mbgl::Observer {
public:
    Observer(MGLObserver *observer_): observer(observer_) {}
    virtual ~Observer() = default;
    virtual void notify(const ObservableEvent& event);
    virtual std::size_t id() const;

protected:

    /// Cocoa observer that this adapter bridges to.
    __weak MGLObserver *observer = nullptr;
};

}
}

#pragma mark -
@interface MGLObserver ()
@property (nonatomic, assign) std::shared_ptr<mbgl::darwin::Observer> peer;

// TODO: Consider making public
@property (nonatomic, copy) void (^notificationHandler)(MGLEvent *);

// Debug property used for development.
@property (nonatomic, assign) BOOL observing;
@end

NS_ASSUME_NONNULL_END

