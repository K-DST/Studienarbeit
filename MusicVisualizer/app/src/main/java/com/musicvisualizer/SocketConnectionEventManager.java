package com.musicvisualizer;

public class SocketConnectionEventManager {
   private static SocketConnectionEventHandler handler;

    public static void setHandler(SocketConnectionEventHandler toSet){
            handler = toSet;
    }

    public static SocketConnectionEventHandler getHandler(){
        return handler;
    }
}
