package org.maplibre.android.plugin

import java.nio.ByteBuffer

class PluginProtocolHandlerResponse {

    fun generateBuffer() {
        data = ByteBuffer.allocateDirect(1000);

        // Example: write bytes
        for (i in 0..<255) {
            data!!.put(i.toByte())
        }

    }

    var data: ByteBuffer? = null;

}
