-dontwarn java.lang.ClassValue
-dontwarn javax.lang.model.element.Modifier
# Keep instrumentation runner and test classes
-keep class org.maplibre.android.InstrumentationRunner { *; }
-keep class org.maplibre.android.** { *; }

# Keep test framework classes
-keep class androidx.test.** { *; }
-keep class android.test.** { *; }
-keep class junit.** { *; }

# Keep annotation classes
-keep class java.lang.annotation.** { *; }
-keep interface java.lang.annotation.** { *; }

# Prevent method ID ordering issues
-optimizations !code/allocation/variable
