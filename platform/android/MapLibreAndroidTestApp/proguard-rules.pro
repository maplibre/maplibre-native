# Mapbox ProGuard configuration is handled in the SDK,
# This file contains test app specific configuration

# Kotlin
-dontnote kotlin.**

# LeakCanary
-dontnote com.squareup.leakcanary.internal.**
-dontnote gnu.trove.THashMap

# GMS
-dontnote com.google.android.gms.**

-keep class com.mapbox.mapboxsdk.testapp.model.customlayer.ExampleCustomLayer { *; }

 # okhttp
-dontwarn org.bouncycastle.jsse.BCSSLSocket
-dontwarn org.bouncycastle.jsse.BCSSLParameters
-dontwarn org.bouncycastle.jsse.provider.BouncyCastleJsseProvider
-dontwarn org.conscrypt.*
-dontwarn org.openjsse.javax.net.ssl.SSLParameters
-dontwarn org.openjsse.javax.net.ssl.SSLSocket
-dontwarn org.openjsse.net.ssl.OpenJSSE
