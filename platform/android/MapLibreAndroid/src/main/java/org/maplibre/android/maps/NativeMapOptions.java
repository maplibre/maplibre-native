package org.maplibre.android.maps;

public class NativeMapOptions {

  private final float pixelRatio;
  private final boolean crossSourceCollisions;

  private final boolean actionJournalEnabled;
  private final String actionJournalPath;
  private final long actionJournalLogFileSize;
  private final long actionJournalLogFileCount;

  public NativeMapOptions(MapLibreMapOptions options) {
    pixelRatio = options.getPixelRatio();
    crossSourceCollisions = options.getCrossSourceCollisions();

    actionJournalEnabled = options.getActionJournalEnabled();
    actionJournalPath = options.getActionJournalPath();
    actionJournalLogFileSize = options.getActionJournalLogFileSize();
    actionJournalLogFileCount = options.getActionJournalLogFileCount();
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
}
