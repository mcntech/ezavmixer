package com.mcntech.udpplayer;

import java.nio.ByteBuffer;

/**
 * Created by ramp on 12/6/17.
 */

public interface AudRenderInterface {
    void Render(int strmId, ByteBuffer buffer, long pts);
}
