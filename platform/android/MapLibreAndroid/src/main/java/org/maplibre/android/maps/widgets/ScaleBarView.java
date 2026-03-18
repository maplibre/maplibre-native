package org.maplibre.android.maps.widgets;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.view.ViewCompat;
import androidx.core.view.ViewPropertyAnimatorCompat;

import java.text.DecimalFormat;
import java.util.Locale;

/**
 * UI element overlaid on a map to show the scale of the map at the current zoom level
 * and latitude. The scale bar updates automatically as the map view changes.
 * <p>
 * You can change the behaviour of this View during initialisation with
 * {@link org.maplibre.android.maps.MapLibreMapOptions}, and xml attributes. While running you can
 * use {@link org.maplibre.android.maps.UiSettings}.
 * </p>
 */
public final class ScaleBarView extends View {

  private static final float FEET_PER_METER = 3.28084f;
  private static final float FEET_PER_MILE = 5280f;
  private static final float METERS_PER_KILOMETER = 1000f;

  // Bar dimensions in dp
  private static final float BAR_HEIGHT_DP = 4f;
  private static final float MIN_BAR_WIDTH_DP = 30f;
  private static final float LABEL_MARGIN_DP = 2f;
  private static final float BORDER_WIDTH_DP = 1f;
  private static final float CORNER_RADIUS_DP = 2f;

  // Metric distance table (distance in meters, number of bars)
  private static final float[][] METRIC_TABLE = {
      {1, 2}, {2, 2}, {4, 2}, {10, 2}, {20, 2}, {50, 2}, {75, 3}, {100, 2},
      {150, 2}, {200, 2}, {300, 3}, {500, 2}, {1000, 2}, {1500, 2}, {3000, 3},
      {5000, 2}, {10000, 2}, {20000, 2}, {30000, 3}, {50000, 2}, {100000, 2},
      {200000, 2}, {300000, 3}, {400000, 2}, {500000, 2}, {600000, 3}, {800000, 2}
  };

  // Imperial distance table (distance in feet, number of bars)
  private static final float[][] IMPERIAL_TABLE = {
      {4, 2}, {6, 2}, {10, 2}, {20, 2}, {30, 2}, {50, 2}, {75, 3}, {100, 2},
      {200, 2}, {300, 3}, {400, 2}, {600, 3}, {800, 2}, {1000, 2},
      {0.25f * FEET_PER_MILE, 2}, {0.5f * FEET_PER_MILE, 2}, {1 * FEET_PER_MILE, 2},
      {2 * FEET_PER_MILE, 2}, {3 * FEET_PER_MILE, 3}, {4 * FEET_PER_MILE, 2},
      {8 * FEET_PER_MILE, 2}, {12 * FEET_PER_MILE, 2}, {15 * FEET_PER_MILE, 3},
      {20 * FEET_PER_MILE, 2}, {30 * FEET_PER_MILE, 3}, {40 * FEET_PER_MILE, 2},
      {80 * FEET_PER_MILE, 2}, {120 * FEET_PER_MILE, 2}, {200 * FEET_PER_MILE, 2},
      {300 * FEET_PER_MILE, 3}, {400 * FEET_PER_MILE, 2}
  };

  // Paint objects
  private final Paint primaryPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
  private final Paint secondaryPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
  private final Paint borderPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
  private final Paint textPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
  private final Paint textStrokePaint = new Paint(Paint.ANTI_ALIAS_FLAG);

  // Dimensions in pixels
  private final float density;
  private final float barHeight;
  private final float minBarWidth;
  private final float labelMargin;
  private final float borderWidth;
  private final float cornerRadius;

  // State
  private double metersPerPixel = 0;
  private boolean usesMetricSystem = true;
  private float currentDistance = 0;
  private int currentNumBars = 2;
  private float currentBarWidth = 0;

  // Colors
  @ColorInt
  private int primaryColor = Color.rgb(18, 45, 17);  // Dark green
  @ColorInt
  private int secondaryColor = Color.rgb(247, 247, 247);  // Light gray

  // Animation
  @Nullable
  private ViewPropertyAnimatorCompat fadeAnimator;

  // Text formatting
  private final DecimalFormat decimalFormat = new DecimalFormat("#.##");
  private final Rect textBounds = new Rect();
  private final RectF barRect = new RectF();

  public ScaleBarView(@NonNull Context context) {
    super(context);
    density = context.getResources().getDisplayMetrics().density;
    barHeight = BAR_HEIGHT_DP * density;
    minBarWidth = MIN_BAR_WIDTH_DP * density;
    labelMargin = LABEL_MARGIN_DP * density;
    borderWidth = BORDER_WIDTH_DP * density;
    cornerRadius = CORNER_RADIUS_DP * density;
    initialize();
  }

  public ScaleBarView(@NonNull Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    density = context.getResources().getDisplayMetrics().density;
    barHeight = BAR_HEIGHT_DP * density;
    minBarWidth = MIN_BAR_WIDTH_DP * density;
    labelMargin = LABEL_MARGIN_DP * density;
    borderWidth = BORDER_WIDTH_DP * density;
    cornerRadius = CORNER_RADIUS_DP * density;
    initialize();
  }

  public ScaleBarView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    density = context.getResources().getDisplayMetrics().density;
    barHeight = BAR_HEIGHT_DP * density;
    minBarWidth = MIN_BAR_WIDTH_DP * density;
    labelMargin = LABEL_MARGIN_DP * density;
    borderWidth = BORDER_WIDTH_DP * density;
    cornerRadius = CORNER_RADIUS_DP * density;
    initialize();
  }

  private void initialize() {
    setEnabled(false);
    setVisibility(View.INVISIBLE);
    setAlpha(0f);

    // Check locale for default metric system
    Locale locale = Locale.getDefault();
    String country = locale.getCountry();
    // US, Myanmar, and Liberia use imperial
    usesMetricSystem = !("US".equals(country) || "MM".equals(country) || "LR".equals(country));

    // Set up paints
    primaryPaint.setStyle(Paint.Style.FILL);
    primaryPaint.setColor(primaryColor);

    secondaryPaint.setStyle(Paint.Style.FILL);
    secondaryPaint.setColor(secondaryColor);

    borderPaint.setStyle(Paint.Style.STROKE);
    borderPaint.setStrokeWidth(borderWidth);
    borderPaint.setColor(primaryColor);

    textPaint.setTextSize(8 * density);
    textPaint.setColor(Color.BLACK);
    textPaint.setTextAlign(Paint.Align.CENTER);

    textStrokePaint.setTextSize(8 * density);
    textStrokePaint.setColor(Color.WHITE);
    textStrokePaint.setTextAlign(Paint.Align.CENTER);
    textStrokePaint.setStyle(Paint.Style.STROKE);
    textStrokePaint.setStrokeWidth(2 * density);
    textStrokePaint.setStrokeJoin(Paint.Join.ROUND);

    // Layout params - wrap content
    ViewGroup.LayoutParams lp = new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.WRAP_CONTENT,
        ViewGroup.LayoutParams.WRAP_CONTENT
    );
    setLayoutParams(lp);
  }

  /**
   * Set whether the scale bar is enabled (visible).
   *
   * @param enabled true to show the scale bar, false to hide it
   */
  @Override
  public void setEnabled(boolean enabled) {
    super.setEnabled(enabled);
    if (enabled) {
      resetAnimation();
      setAlpha(1.0f);
      setVisibility(View.VISIBLE);
      recalculate();
    } else {
      resetAnimation();
      setAlpha(0.0f);
      setVisibility(View.INVISIBLE);
    }
  }

  private void resetAnimation() {
    if (fadeAnimator != null) {
      fadeAnimator.cancel();
      fadeAnimator = null;
    }
  }

  /**
   * Update the scale bar with the current meters per pixel at the map center.
   *
   * @param metersPerPixel meters per pixel at the current latitude and zoom
   */
  public void update(double metersPerPixel) {
    if (this.metersPerPixel == metersPerPixel) {
      return;
    }
    this.metersPerPixel = metersPerPixel;

    if (!isEnabled()) {
      return;
    }

    recalculate();
  }

  private void recalculate() {
    if (metersPerPixel <= 0) {
      fadeOut();
      return;
    }

    float maxWidth = getMaxBarWidth();
    if (maxWidth <= 0) {
      return;
    }

    float unitsPerPixel = usesMetricSystem ? (float) metersPerPixel : (float) metersPerPixel * FEET_PER_METER;
    float maxDistance = maxWidth * unitsPerPixel;

    float[][] table = usesMetricSystem ? METRIC_TABLE : IMPERIAL_TABLE;

    // Find the best row from the table
    float distance = table[0][0];
    int numBars = (int) table[0][1];

    for (float[] row : table) {
      if (row[0] > maxDistance) {
        break;
      }
      distance = row[0];
      numBars = (int) row[1];
    }

    // Check if the scale is out of range
    float lastDistance = table[table.length - 1][0];
    if (maxDistance > lastDistance) {
      fadeOut();
      return;
    }

    // Calculate bar width
    float barWidth = distance / unitsPerPixel;
    if (barWidth < minBarWidth) {
      fadeOut();
      return;
    }

    // Round to whole pixels per segment
    barWidth = numBars * (float) Math.floor(barWidth / numBars);

    // Update state
    currentDistance = distance;
    currentNumBars = numBars;
    currentBarWidth = barWidth;

    // Ensure visible
    if (getAlpha() < 1.0f) {
      resetAnimation();
      fadeAnimator = ViewCompat.animate(this).alpha(1.0f).setDuration(200);
      setVisibility(View.VISIBLE);
    }

    requestLayout();
    invalidate();
  }

  private void fadeOut() {
    if (getAlpha() > 0f && fadeAnimator == null) {
      fadeAnimator = ViewCompat.animate(this).alpha(0.0f).setDuration(200);
    }
  }

  private float getMaxBarWidth() {
    View parent = (View) getParent();
    if (parent == null) {
      return 0;
    }
    return parent.getWidth() / 2f;
  }

  @Override
  protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (currentBarWidth <= 0) {
      setMeasuredDimension(0, 0);
      return;
    }

    // Calculate label text bounds for the final label
    String lastLabel = formatDistance(currentDistance);
    textPaint.getTextBounds(lastLabel, 0, lastLabel.length(), textBounds);

    int width = (int) Math.ceil(currentBarWidth + textBounds.width() / 2f + labelMargin);
    int height = (int) Math.ceil(textBounds.height() + labelMargin + barHeight);

    setMeasuredDimension(width, height);
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);

    if (currentBarWidth <= 0 || currentNumBars <= 0) {
      return;
    }

    boolean rtl = ViewCompat.getLayoutDirection(this) == ViewCompat.LAYOUT_DIRECTION_RTL;

    // Calculate positions
    float lastLabelText = formatDistance(currentDistance).length();
    textPaint.getTextBounds(formatDistance(currentDistance), 0, (int) lastLabelText, textBounds);
    float labelHeight = textBounds.height();

    float barTop = labelHeight + labelMargin;
    float barLeft = rtl ? textBounds.width() / 2f : 0;
    float segmentWidth = currentBarWidth / currentNumBars;

    // Draw container background with rounded corners
    barRect.set(barLeft, barTop, barLeft + currentBarWidth, barTop + barHeight);
    canvas.drawRoundRect(barRect, cornerRadius, cornerRadius, secondaryPaint);

    // Draw individual bar segments
    for (int i = 0; i < currentNumBars; i++) {
      Paint paint = (i % 2 == 0) ? primaryPaint : secondaryPaint;
      float segmentLeft = barLeft + (rtl ? currentBarWidth - (i + 1) * segmentWidth : i * segmentWidth);
      float segmentRight = segmentLeft + segmentWidth;

      // For first and last segments, use rounded corners
      if (i == 0 || i == currentNumBars - 1) {
        barRect.set(segmentLeft, barTop, segmentRight, barTop + barHeight);
        canvas.save();
        if (i == 0 && !rtl || i == currentNumBars - 1 && rtl) {
          // Left rounded
          canvas.clipRect(segmentLeft, barTop, segmentLeft + cornerRadius, barTop + barHeight);
          canvas.drawRoundRect(barRect, cornerRadius, cornerRadius, paint);
          canvas.restore();
          canvas.drawRect(segmentLeft + cornerRadius, barTop, segmentRight, barTop + barHeight, paint);
        } else {
          // Right rounded
          canvas.clipRect(segmentRight - cornerRadius, barTop, segmentRight, barTop + barHeight);
          canvas.drawRoundRect(barRect, cornerRadius, cornerRadius, paint);
          canvas.restore();
          canvas.drawRect(segmentLeft, barTop, segmentRight - cornerRadius, barTop + barHeight, paint);
        }
      } else {
        canvas.drawRect(segmentLeft, barTop, segmentRight, barTop + barHeight, paint);
      }
    }

    // Draw border
    barRect.set(barLeft, barTop, barLeft + currentBarWidth, barTop + barHeight);
    canvas.drawRoundRect(barRect, cornerRadius, cornerRadius, borderPaint);

    // Draw labels
    float labelY = labelHeight;
    float distancePerSegment = currentDistance / currentNumBars;

    for (int i = 0; i <= currentNumBars; i++) {
      float labelDistance = distancePerSegment * i;
      String label = formatDistance(labelDistance);

      float labelX;
      if (rtl) {
        labelX = barLeft + currentBarWidth - i * segmentWidth;
      } else {
        labelX = barLeft + i * segmentWidth;
      }

      // Draw text stroke (outline) first, then fill
      canvas.drawText(label, labelX, labelY, textStrokePaint);
      canvas.drawText(label, labelX, labelY, textPaint);
    }
  }

  private String formatDistance(float distance) {
    if (distance == 0) {
      return "0";
    }

    if (usesMetricSystem) {
      if (distance >= METERS_PER_KILOMETER) {
        float km = distance / METERS_PER_KILOMETER;
        return decimalFormat.format(km) + " km";
      } else {
        return decimalFormat.format(distance) + " m";
      }
    } else {
      if (distance >= FEET_PER_MILE) {
        float miles = distance / FEET_PER_MILE;
        return decimalFormat.format(miles) + " mi";
      } else {
        return decimalFormat.format(distance) + " ft";
      }
    }
  }

  // Public API

  /**
   * Returns whether the scale bar uses the metric system.
   *
   * @return true if using metric (meters/kilometers), false if using imperial (feet/miles)
   */
  public boolean isMetric() {
    return usesMetricSystem;
  }

  /**
   * Sets whether the scale bar should use the metric system.
   *
   * @param metric true to use metric (meters/kilometers), false to use imperial (feet/miles)
   */
  public void setMetric(boolean metric) {
    if (this.usesMetricSystem != metric) {
      this.usesMetricSystem = metric;
      recalculate();
    }
  }

  /**
   * Returns the primary color of the scale bar.
   *
   * @return the primary color
   */
  @ColorInt
  public int getPrimaryColor() {
    return primaryColor;
  }

  /**
   * Sets the primary color of the scale bar (odd-numbered bars and border).
   *
   * @param color the color to use
   */
  public void setPrimaryColor(@ColorInt int color) {
    this.primaryColor = color;
    primaryPaint.setColor(color);
    borderPaint.setColor(color);
    invalidate();
  }

  /**
   * Returns the secondary color of the scale bar.
   *
   * @return the secondary color
   */
  @ColorInt
  public int getSecondaryColor() {
    return secondaryColor;
  }

  /**
   * Sets the secondary color of the scale bar (even-numbered bars and background).
   *
   * @param color the color to use
   */
  public void setSecondaryColor(@ColorInt int color) {
    this.secondaryColor = color;
    secondaryPaint.setColor(color);
    invalidate();
  }

  /**
   * Sets the text color for the scale bar labels.
   *
   * @param color the color to use for label text
   */
  public void setTextColor(@ColorInt int color) {
    textPaint.setColor(color);
    invalidate();
  }

  /**
   * Gets the text color for the scale bar labels.
   *
   * @return the label text color
   */
  @ColorInt
  public int getTextColor() {
    return textPaint.getColor();
  }

  /**
   * Refresh the scale bar layout. Call this after configuration changes.
   */
  public void refresh() {
    recalculate();
  }
}
