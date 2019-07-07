package com.yyt.anti_debug;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals("android.intent.action.BOOT_COMPLETED")){
            Log.d("TEST","boot receiver!!");

            Intent i = new Intent(context,OwnService.class);
            context.startService(i);
        }
    }
}
