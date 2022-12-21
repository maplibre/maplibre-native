package android.app;

import android.util.Log;

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@LargeTest
@RunWith(AndroidJUnit4ClassRunner.class)
public class NativeActivityTest {

    @Rule
    public ActivityTestRule<NativeActivity> mActivityTestRule = new ActivityTestRule<>(NativeActivity.class, false, false);

    @Test(timeout = 1200000L)
    public void runBenchmark() throws Exception {
        Log.v("Benchmark", "Start the benchmark");
        mActivityTestRule.launchActivity(null);
        while (TestState.running) {
            Log.v("Benchmark", "Benchmark is running...");
            Thread.sleep(1000L);
        }
        Log.v("Benchmark", "All benchmarks are finished!");
        mActivityTestRule.finishActivity();
        Assert.assertTrue("Benchmark was successfully finished", TestState.testResult);
    }
}
