package org.maplibre.android.maps.renderer.surfaceview;

import android.content.Context;
import androidx.annotation.NonNull;

public class WebGPUSurfaceViewMapRenderer extends SurfaceViewMapRenderer {

  public WebGPUSurfaceViewMapRenderer(Context context,
                                @NonNull MapLibreWebGPUSurfaceView surfaceView,
                                String localIdeographFontFamily) {
    super(context, surfaceView, localIdeographFontFamily);

    this.surfaceView.setRenderer(this);
  }

}
