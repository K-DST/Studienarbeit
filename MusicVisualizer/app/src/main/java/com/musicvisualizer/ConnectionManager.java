package com.musicvisualizer;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.ParcelUuid;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class ConnectionManager implements Runnable {
    static ConnectionManager cm = null;
    BluetoothAdapter myBluetooth = BluetoothAdapter.getDefaultAdapter();
    BluetoothDevice hc05 = myBluetooth.getRemoteDevice("98:D3:71:FD:38:CC");
    ParcelUuid[] uuids = hc05.getUuids();
    BluetoothSocket socket;

    private ConnectionManager(){}

    public static ConnectionManager getInstance(){
        if(cm == null){
            cm = new ConnectionManager();
        }
        return cm;
    }

    public void checkConnection(){
        Thread t1 = new Thread( cm );
        t1.start();
    }

    public void listAllDevices(){
        Set<BluetoothDevice> pairedDevices = myBluetooth.getBondedDevices();

        List<String> s = new ArrayList<String>();
        for(BluetoothDevice bt : pairedDevices)
            s.add(bt.getName());
        Log.d("Device List", s.toString());
    }

    public void sendString(String s, Activity a){
        Toast.makeText(a, s, Toast.LENGTH_LONG).show();
    }

    public void sendString(String s) {
        if(socket != null || socket.isConnected()) {
            try {
                OutputStream outputStream = socket.getOutputStream();
                outputStream.write("!".getBytes());
                try {
                    Thread.sleep(500);
                }catch (InterruptedException e){}
                outputStream.write((s.getBytes()));

            }catch (IllegalStateException e1){
                socket = null;
                SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
                listener.onDisconnect();

                Log.e("Connection", "unsuccessful");
            }catch (IOException e2){
                socket = null;
                SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
                listener.onDisconnect();

                Log.e("Connection", "unsuccessful");
            }catch (NullPointerException e3){
                socket = null;
                SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
                listener.onDisconnect();

                Log.e("Connection", "unsuccessful");
            }
        }
    }

    @Override
    public void run() {
        connectionProcedure();
    }

    public void connectionProcedure(){
        try {
            if(socket == null || !socket.isConnected()) {
                socket = hc05.createInsecureRfcommSocketToServiceRecord(uuids[0].getUuid());
                myBluetooth.cancelDiscovery();
                socket.connect();//start connection
            }
            socket.getOutputStream().write("x".getBytes());
            try{
                Thread.sleep(500);
            }catch(InterruptedException e){}
            socket.getOutputStream().write("5".getBytes());
            SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
            listener.onConnect();
            Log.d("Connection", "successful");
        }catch (IllegalStateException e1){
            socket = null;
            SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
            listener.onDisconnect();

            Log.e("Connection", "unsuccessful");
        }catch (IOException e2){
            socket = null;
            SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
            listener.onDisconnect();

            Log.e("Connection", "unsuccessful");
        }catch (NullPointerException e3){
            socket = null;
            SocketConnectionEventHandler listener = SocketConnectionEventManager.getHandler();
            listener.onDisconnect();

            Log.e("Connection", "unsuccessful");
        }
    }
}
