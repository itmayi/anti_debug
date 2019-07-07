package com.yyt.anti_debug;

import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class MainActivity extends AppCompatActivity {


    private String exe_path = "/data/local/tmp/mprop";
    private File exe_file;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());

//        exe_file = new File(exe_path);
//        exe_file.setExecutable(true);
//        execCmd(exe_path);

        Toast.makeText(getApplicationContext(),"ro.debuggable=" + stringFromJNI(),Toast.LENGTH_LONG).show();

    }

    public native String stringFromJNI();

    private void execCmd(String cmd){
        Runtime runtime = Runtime.getRuntime();
        try {
            Process p = runtime.exec(cmd);
            InputStream is = p.getInputStream();
            InputStreamReader isr = new InputStreamReader(is);
            BufferedReader br = new BufferedReader(isr);
            String line = null;
            while (null != (line = br.readLine())){
                Log.e("++++++++++++",line);
            }

        } catch (IOException e) {

            Log.e("+++++++","exec failed !!!!!");
            e.printStackTrace();
        }

    }

}
