package org.maplibre.android.plugin

import java.nio.ByteBuffer
import java.nio.charset.StandardCharsets

class PluginProtocolHandlerResponse {

    fun generateBuffer(stringBuffer: String) {
        val byteArray = stringBuffer.toByteArray(StandardCharsets.UTF_8);
        val buffer = ByteBuffer.allocateDirect(byteArray.size);
        buffer.put(byteArray);
        buffer.flip();
        data = buffer;
    }

    var data: ByteBuffer? = null;

}
