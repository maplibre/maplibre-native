package org.maplibre.android.attribution

import android.graphics.Bitmap
import android.graphics.PointF
import android.widget.TextView

class AttributionMeasure internal constructor(
    private val snapshot: Bitmap,
    private val logo: Bitmap,
    private val logoSmall: Bitmap,
    private val textViewLong: TextView,
    private val textViewShort: TextView,
    private val margin: Float,
) {
    private var shorterText = false

    fun measure(): AttributionLayout? {
        val chain =
            Chain(
                FullLogoLongTextCommand(),
                FullLogoShortTextCommand(),
                SmallLogoLongTextCommand(),
                SmallLogoShortTextCommand(),
                LongTextCommand(),
                ShortTextCommand(),
                NoTextCommand(),
            )

        val attributionLayout = chain.start(this)
        shorterText = attributionLayout!!.isShortText
        return attributionLayout
    }

    val textView: TextView
        get() = if (shorterText) textViewShort else textViewLong

    private class FullLogoLongTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.logoContainerWidth + measure.textViewContainerWidth
            val fitBounds = width <= measure.maxSize
            if (fitBounds) {
                val anchor = calculateAnchor(measure.snapshot, measure.textViewLong, measure.margin)
                return AttributionLayout(measure.logo, anchor, false)
            }
            return null
        }
    }

    private class FullLogoShortTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.logoContainerWidth + measure.textViewShortContainerWidth
            val fitBounds = width <= measure.maxSizeShort
            if (fitBounds) {
                val anchor = calculateAnchor(measure.snapshot, measure.textViewShort, measure.margin)
                return AttributionLayout(measure.logo, anchor, true)
            }
            return null
        }
    }

    private class SmallLogoLongTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.logoSmallContainerWidth + measure.textViewContainerWidth
            val fitBounds = width <= measure.maxSize
            if (fitBounds) {
                val anchor = calculateAnchor(measure.snapshot, measure.textViewLong, measure.margin)
                return AttributionLayout(measure.logoSmall, anchor, false)
            }
            return null
        }
    }

    private class SmallLogoShortTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.logoContainerWidth + measure.textViewShortContainerWidth
            val fitBounds = width <= measure.maxSizeShort
            if (fitBounds) {
                val anchor = calculateAnchor(measure.snapshot, measure.textViewShort, measure.margin)
                return AttributionLayout(measure.logoSmall, anchor, true)
            }
            return null
        }
    }

    private class LongTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.textViewContainerWidth + measure.margin
            val fitBounds = width <= measure.maxSize
            if (fitBounds) {
                return AttributionLayout(
                    null,
                    calculateAnchor(measure.snapshot, measure.textViewLong, measure.margin),
                    false,
                )
            }
            return null
        }
    }

    private class ShortTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout? {
            val width = measure.textViewShortContainerWidth + measure.margin
            val fitBounds = width <= measure.maxSizeShort
            if (fitBounds) {
                val anchor = calculateAnchor(measure.snapshot, measure.textViewShort, measure.margin)
                return AttributionLayout(null, anchor, true)
            }
            return null
        }
    }

    private class NoTextCommand : Command {
        override fun execute(measure: AttributionMeasure): AttributionLayout = AttributionLayout(null, null, false)
    }

    private class Chain(
        vararg commands: Command,
    ) {
        private val commands: List<Command> = commands.asList()

        fun start(measure: AttributionMeasure): AttributionLayout? {
            var attributionLayout: AttributionLayout? = null
            for (command in commands) {
                attributionLayout = command.execute(measure)
                if (attributionLayout != null) {
                    break
                }
            }
            return attributionLayout
        }
    }

    interface Command {
        fun execute(measure: AttributionMeasure): AttributionLayout?
    }

    private val textViewContainerWidth: Float
        get() = textViewLong.measuredWidth + margin

    private val logoContainerWidth: Float
        get() = logo.width + (2 * margin)

    private val textViewShortContainerWidth: Float
        get() = textViewShort.measuredWidth + margin

    private val logoSmallContainerWidth: Float
        get() = logoSmall.width + (2 * margin)

    private val maxSize: Float
        get() = (snapshot.width * 8 / 10).toFloat()

    private val maxSizeShort: Float
        get() = snapshot.width.toFloat()

    class Builder {
        private var snapshot: Bitmap? = null
        private var logo: Bitmap? = null
        private var logoSmall: Bitmap? = null
        private var textView: TextView? = null
        private var textViewShort: TextView? = null
        private var marginPadding = 0f

        fun setSnapshot(snapshot: Bitmap): Builder {
            this.snapshot = snapshot
            return this
        }

        fun setLogo(logo: Bitmap): Builder {
            this.logo = logo
            return this
        }

        fun setLogoSmall(logoSmall: Bitmap): Builder {
            this.logoSmall = logoSmall
            return this
        }

        fun setTextView(textView: TextView): Builder {
            this.textView = textView
            return this
        }

        fun setTextViewShort(textViewShort: TextView): Builder {
            this.textViewShort = textViewShort
            return this
        }

        fun setMarginPadding(marginPadding: Float): Builder {
            this.marginPadding = marginPadding
            return this
        }

        fun build(): AttributionMeasure =
            AttributionMeasure(
                snapshot!!,
                logo!!,
                logoSmall!!,
                textView!!,
                textViewShort!!,
                marginPadding,
            )
    }

    private companion object {
        fun calculateAnchor(
            snapshot: Bitmap,
            textView: TextView,
            margin: Float,
        ): PointF =
            PointF(
                snapshot.width - textView.measuredWidth - margin,
                snapshot.height - margin - textView.measuredHeight,
            )
    }
}
