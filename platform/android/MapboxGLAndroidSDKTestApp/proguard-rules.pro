# Mapbox ProGuard configuration is handled in the SDK,
# This file contains test app specific configuration

# Kotlin
-dontnote kotlin.**

# LeakCanary
-dontnote com.squareup.leakcanary.internal.**
-dontnote gnu.trove.THashMap

# GMS
-dontnote com.google.android.gms.**

-keep class org.maplibre.android.testapp.model.customlayer.ExampleCustomLayer { *; }
