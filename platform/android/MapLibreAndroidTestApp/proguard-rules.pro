# Mapbox ProGuard configuration is handled in the SDK,
# This file contains test app specific configuration

# Kotlin
-dontnote kotlin.**
-keep class kotlin.LazyKt { *; }
-keep class kotlin.LazyKt__LazyJVMKt { *; }
-keep class kotlin.LazyKt__LazyKt { *; }

# LeakCanary
-dontnote com.squareup.leakcanary.internal.**
-dontnote gnu.trove.THashMap

# GMS
-dontnote com.google.android.gms.**
-dontwarn com.google.android.gms.**
-keep class com.google.android.gms.** { *; }

-keep class org.maplibre.android.testapp.model.customlayer.ExampleCustomLayer { *; }

-keep class androidx.tracing.** { *; }

 # okhttp
-dontwarn org.bouncycastle.jsse.BCSSLSocket
-dontwarn org.bouncycastle.jsse.BCSSLParameters
-dontwarn org.bouncycastle.jsse.provider.BouncyCastleJsseProvider
-dontwarn org.conscrypt.*
-dontwarn org.openjsse.javax.net.ssl.SSLParameters
-dontwarn org.openjsse.javax.net.ssl.SSLSocket
-dontwarn org.openjsse.net.ssl.OpenJSSE
