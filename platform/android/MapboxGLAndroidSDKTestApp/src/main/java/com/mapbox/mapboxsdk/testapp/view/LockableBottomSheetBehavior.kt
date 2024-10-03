package org.maplibre.android.testapp.view

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import androidx.coordinatorlayout.widget.CoordinatorLayout
import com.google.android.material.bottomsheet.BottomSheetBehavior

class LockableBottomSheetBehavior<V : View?>(context: Context, attrs: AttributeSet?) :
    BottomSheetBehavior<V>(context, attrs) {
    private var locked = false
    fun setLocked(locked: Boolean) {
        this.locked = locked
    }

    override fun onInterceptTouchEvent(
        parent: CoordinatorLayout,
        child: V & Any,
        event: MotionEvent
    ): Boolean {
        var handled = false
        if (!locked) {
            handled = super.onInterceptTouchEvent(parent, child, event)
        }
        return handled
    }

    override fun onTouchEvent(
        parent: CoordinatorLayout,
        child: V & Any,
        event: MotionEvent
    ): Boolean {
        var handled = false
        if (!locked) {
            handled = super.onTouchEvent(parent, child, event)
        }
        return handled
    }

    override fun onStartNestedScroll(
        coordinatorLayout: CoordinatorLayout,
        child: V & Any,
        directTargetChild: View,
        target: View,
        nestedScrollAxes: Int
    ): Boolean {
        var handled = false
        if (!locked) {
            handled = super.onStartNestedScroll(
                coordinatorLayout,
                child,
                directTargetChild,
                target,
                nestedScrollAxes
            )
        }
        return handled
    }

    override fun onNestedPreScroll(
        coordinatorLayout: CoordinatorLayout,
        child: V & Any,
        target: View,
        dx: Int,
        dy: Int,
        consumed: IntArray
    ) {
        if (!locked) {
            super.onNestedPreScroll(coordinatorLayout, child, target, dx, dy, consumed)
        }
    }

    override fun onStopNestedScroll(
        coordinatorLayout: CoordinatorLayout,
        child: V & Any,
        target: View
    ) {
        if (!locked) {
            super.onStopNestedScroll(coordinatorLayout, child, target)
        }
    }

    override fun onNestedPreFling(
        coordinatorLayout: CoordinatorLayout,
        child: V & Any,
        target: View,
        velocityX: Float,
        velocityY: Float
    ): Boolean {
        var handled = false
        if (!locked) {
            handled = super.onNestedPreFling(coordinatorLayout, child, target, velocityX, velocityY)
        }
        return handled
    }
}
