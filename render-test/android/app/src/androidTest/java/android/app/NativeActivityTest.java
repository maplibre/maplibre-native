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
    public void runRenderTests() throws Exception {
        Log.v("Test", "Start the test");
        mActivityTestRule.launchActivity(null);
        while (TestState.running) {
            Log.v("Test", "Test is running");
            Thread.sleep(1000L);
        }
        Log.v("Test", "All render tests are finished!");
        Assert.assertTrue("All test cases are passed", TestState.testResult);

    }
}