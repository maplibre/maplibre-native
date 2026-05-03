package android.app;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.Gravity;
import android.view.WindowManager;
import android.widget.TextView;

public class RenderTestActivity extends NativeActivity {

    private TextView progressTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        progressTextView = new TextView(this);
        progressTextView.setText("Starting tests...");
        progressTextView.setTextSize(18f);
        progressTextView.setTextColor(0xFFFFFFFF);
        progressTextView.setShadowLayer(2f, 1f, 1f, 0xFF000000);
        progressTextView.setPadding(24, 24, 24, 24);
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE,
            PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.token = getWindow().getDecorView().getWindowToken();

        getWindowManager().addView(progressTextView, params);
    }

    @Override
    public void onDetachedFromWindow() {
        if (progressTextView != null) {
            getWindowManager().removeView(progressTextView);
        }
        super.onDetachedFromWindow();
    }

    /**
     * Called from C++ via JNI to update the progress display.
     */
    public void updateProgress(final int completed, final int total) {
        runOnUiThread(() -> {
            if (progressTextView != null) {
                progressTextView.setText(completed + " / " + total + " tests complete");
            }
        });
    }
}
