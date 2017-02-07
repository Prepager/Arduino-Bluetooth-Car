package com.teknikfag.bil;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.os.Handler;

import android.util.Log;
import android.content.Intent;

import android.view.View;
import android.view.MotionEvent;

import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothSocket;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.os.AsyncTask;

import java.io.IOException;
import java.util.UUID;

public class carController extends ActionBarActivity {

    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // Elements
    private SeekBar forwardSpeed;

    private Button hazardButton;

    private Button disconnectButton;

    private Button backwardButton;

    private Button leftBlinkButton;
    private Button leftTurnButton;

    private Button rightBlinkButton;
    private Button rightTurnButton;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS);

        setContentView(R.layout.activity_car_controller);

        new ConnectBT().execute();

        // Alive
        final Handler h = new Handler();
        h.postDelayed(new Runnable()
        {
            private long time = 0;

            @Override
            public void run()
            {
                time += 1000;

                if (btSocket == null || isBtConnected) {
                    sendBluetoothSignal("X");
                }

                h.postDelayed(this, 1000);
            }
        }, 1000);

        // Forward
        forwardSpeed = (SeekBar) findViewById(R.id.speed);
        forwardSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                seekBar.setProgress(0);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                sendBluetoothSignal(Integer.toString(progress));
            }
        });

        // Backwards
        backwardButton = (Button) findViewById(R.id.backwards);
        backwardButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    sendBluetoothSignal("S");
                }else if(event.getAction() == MotionEvent.ACTION_UP){
                    sendBluetoothSignal("G");
                }

                return false;
            }
        });

        // leftBlinkButton
        leftBlinkButton = (Button) findViewById(R.id.blinkLeft);
        leftBlinkButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    sendBluetoothSignal("Q");
                }else if(event.getAction() == MotionEvent.ACTION_UP){
                    sendBluetoothSignal("R");
                }

                return false;
            }
        });

        // rightBlinkButton
        rightBlinkButton = (Button) findViewById(R.id.blinkRight);
        rightBlinkButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    sendBluetoothSignal("E");
                }else if(event.getAction() == MotionEvent.ACTION_UP){
                    sendBluetoothSignal("Y");
                }

                return false;
            }
        });

        // leftTurnButton
        leftTurnButton = (Button) findViewById(R.id.turnLeft);
        leftTurnButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    sendBluetoothSignal("A");
                }else if(event.getAction() == MotionEvent.ACTION_UP){
                    sendBluetoothSignal("F");
                }

                return false;
            }
        });

        // rightTurnButton
        rightTurnButton = (Button) findViewById(R.id.turnRight);
        rightTurnButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    sendBluetoothSignal("D");
                }else if(event.getAction() == MotionEvent.ACTION_UP){
                    sendBluetoothSignal("H");
                }

                return false;
            }
        });

        // hazardButton
        hazardButton = (Button) findViewById(R.id.hazard);
        hazardButton.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                sendBluetoothSignal("C");
            }
        });

        // Disconnect
        disconnectButton = (Button) findViewById(R.id.disconnect);
        disconnectButton.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                h.removeCallbacksAndMessages(null);
                Disconnect();
            }
        });
    }


    private void sendBluetoothSignal(String signal)
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write(signal.toString().getBytes());
            }
            catch (IOException e)
            {
                //Disconnect();
                //msg("Error");
            }
        }
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                btSocket.close(); //close connection
            }
            catch (IOException e)
            {
                msg("Couldn't disconnect...");
            }
        }
        finish(); //return to the first layout

    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true; //if it's here, it's almost connected

        @Override
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(carController.this, "Connecting...", "Please wait...");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }
}
