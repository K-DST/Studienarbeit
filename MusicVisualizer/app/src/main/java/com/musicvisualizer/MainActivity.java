package com.musicvisualizer;

import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements SocketConnectionEventHandler {
    boolean connected = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        SocketConnectionEventManager.setHandler(this);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        FormatUI.getInstance().initializeRadioGroup(this);

        connected = FormatUI.getInstance().changeConnectionStateTo(false, this);
    }

    public void onPowerButtonPress(View view){
        if(connected){
            ConnectionManager.getInstance().sendString("!");
            //ConnectionManager.getInstance().sendString("!", this);
            connected = FormatUI.getInstance().changeConnectionStateTo(false, this);
        }else {
            ConnectionManager.getInstance().checkConnection();
        }
    }

    @Override
    public void onConnect() {
        if(!connected){
            connected = FormatUI.getInstance().changeConnectionStateTo(true, this);
        }
    }

    @Override
    public void onDisconnect() {
        if(connected){
            connected = FormatUI.getInstance().changeConnectionStateTo(false, this);
        }else{
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(getApplicationContext(),"Verbindung konnte nicht hergestellt werden",Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

    public void onColorButtonPress(View view) {
        FormatUI.getInstance().buildColorPickerDialog(view, this);
    }

    public void reset(View view) {
        ConnectionManager.getInstance().sendString("x");
    }
}
