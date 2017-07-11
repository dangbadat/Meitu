package com.xinlan.meitu;

import android.graphics.Bitmap;

/**
 * Created by panyi on 2017/7/10.
 */

public class Tools {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static native void handleSmooth(Bitmap bitmap,float smoothValue);

    public static native void handleWhiteSkin(Bitmap bitmap,float whiteValue);

    public static native void handleSmoothAndWhiteSkin(Bitmap bitmap,float smoothValue,float whiteValue);
}
