package org.maplibre.android.testapp.utils

import android.view.View
import androidx.recyclerview.widget.RecyclerView
import org.maplibre.android.testapp.R

class ItemClickSupport private constructor(private val recyclerView: RecyclerView) {
    private var onItemClickListener: OnItemClickListener? = null
    private var onItemLongClickListener: OnItemLongClickListener? = null
    private val onClickListener = View.OnClickListener { view ->
        onItemClickListener?.let {
            val holder = recyclerView.getChildViewHolder(view)
            it.onItemClicked(recyclerView, holder.adapterPosition, view)
        }
    }
    private val onLongClickListener = View.OnLongClickListener { view ->
        onItemLongClickListener?.let {
            val holder = recyclerView.getChildViewHolder(view)
            it.onItemLongClicked(recyclerView, holder.adapterPosition, view)
        }
        false
    }
    private val attachListener = object : RecyclerView.OnChildAttachStateChangeListener {
        override fun onChildViewAttachedToWindow(view: View) {
            onItemClickListener?.let {
                view.setOnClickListener(onClickListener)
            }
            onItemLongClickListener?.let {
                view.setOnLongClickListener(onLongClickListener)
            }
        }

        override fun onChildViewDetachedFromWindow(view: View) {}
    }

    fun setOnItemClickListener(listener: OnItemClickListener?): ItemClickSupport {
        onItemClickListener = listener
        return this
    }

    fun setOnItemLongClickListener(listener: OnItemLongClickListener?): ItemClickSupport {
        onItemLongClickListener = listener
        return this
    }

    private fun detach(view: RecyclerView) {
        view.removeOnChildAttachStateChangeListener(attachListener)
        view.setTag(R.id.item_click_support, null)
    }

    companion object {
        fun addTo(view: RecyclerView): ItemClickSupport {
            var support = view.getTag(R.id.item_click_support) as ItemClickSupport?
            if (support == null) {
                support = ItemClickSupport(view)
                view.setTag(R.id.item_click_support, support)
                view.addOnChildAttachStateChangeListener(support.attachListener)
            }
            return support
        }

        fun removeFrom(view: RecyclerView): ItemClickSupport? {
            val support = view.getTag(R.id.item_click_support) as ItemClickSupport?
            support?.let {
                it.detach(view)
            }
            return support
        }
    }

    interface OnItemClickListener {
        fun onItemClicked(recyclerView: RecyclerView?, position: Int, view: View?)
    }

    interface OnItemLongClickListener {
        fun onItemLongClicked(recyclerView: RecyclerView?, position: Int, view: View?): Boolean
    }
}
