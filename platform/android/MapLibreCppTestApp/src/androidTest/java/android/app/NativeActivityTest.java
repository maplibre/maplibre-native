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
    public void runUnitTest() throws Exception {
        Log.v("UnitTestRunner", "Start the unit test");
        mActivityTestRule.launchActivity(null);
        while (TestState.running) {
            Log.v("UnitTestRunner", "Unit test is running...");
            Thread.sleep(1000L);
        }
        Log.v("UnitTestRunner", "All unit tests are finished!");
        mActivityTestRule.finishActivity();
        Assert.assertTrue("UnitTestRunner was not successfully finished", TestState.testResult);
    }
}
