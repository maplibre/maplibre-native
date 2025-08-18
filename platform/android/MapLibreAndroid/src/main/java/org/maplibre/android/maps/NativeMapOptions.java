package org.maplibre.android.maps;

public class NativeMapOptions {

  private final float pixelRatio;
  private final boolean crossSourceCollisions;

  private final boolean actionJournalEnabled;
  private final String actionJournalPath;
  private final long actionJournalLogFileSize;
  private final long actionJournalLogFileCount;
  private final int actionJournalRenderingReportInterval;

  public NativeMapOptions(MapLibreMapOptions options) {
    pixelRatio = options.getPixelRatio();
    crossSourceCollisions = options.getCrossSourceCollisions();

    actionJournalEnabled = options.getActionJournalEnabled();
    actionJournalPath = options.getActionJournalPath();
    actionJournalLogFileSize = options.getActionJournalLogFileSize();
    actionJournalLogFileCount = options.getActionJournalLogFileCount();
    actionJournalRenderingReportInterval = options.getActionJournalRenderingReportInterval();
  }

  public NativeMapOptions(float pixelRatio, boolean crossSourceCollisions) {
    this.pixelRatio = pixelRatio;
    this.crossSourceCollisions = crossSourceCollisions;

    actionJournalEnabled = false;
    actionJournalPath = "";
    actionJournalLogFileSize = 0;
    actionJournalLogFileCount = 0;
    actionJournalRenderingReportInterval = 0;
  }

  public float pixelRatio() {
    return pixelRatio;
  }

  public boolean crossSourceCollisions() {
    return crossSourceCollisions;
  }

  public boolean actionJournalEnabled() {
    return actionJournalEnabled;
  }

  public String actionJournalPath() {
    return actionJournalPath;
  }

  public long actionJournalLogFileSize() {
    return actionJournalLogFileSize;
  }

  public long actionJournalLogFileCount() {
    return actionJournalLogFileCount;
  }

  public int actionJournalRenderingReportInterval() {
    return actionJournalRenderingReportInterval;
  }
}
