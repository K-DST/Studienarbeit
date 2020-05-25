package com.musicvisualizer;

import android.app.Activity;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import androidx.core.content.ContextCompat;

import com.flask.colorpicker.ColorPickerView;
import com.flask.colorpicker.builder.ColorPickerClickListener;
import com.flask.colorpicker.builder.ColorPickerDialogBuilder;

public class FormatUI {
    static FormatUI fUi = null;

    private FormatUI(){
    }

    public static FormatUI getInstance(){
        if(fUi == null){
            fUi = new FormatUI();
        }
        return fUi;
    }

    public void initializeRadioGroup(final Activity activity) {
        RadioGroup rg = (RadioGroup) activity.findViewById(R.id.rGroup);
        final Button colorBtnVu = activity.findViewById(R.id.colorBtnVu);
        final Button colorBtnStrip = activity.findViewById(R.id.colorBtnStrip);
        final Button colorBtnLamp = activity.findViewById(R.id.colorBtnLamp);

        colorBtnVu.setTag(ContextCompat.getColor(activity, R.color.colorPickerInitialValue));
        colorBtnStrip.setTag(ContextCompat.getColor(activity, R.color.colorPickerInitialValue));
        colorBtnLamp.setTag(ContextCompat.getColor(activity, R.color.colorPickerInitialValue));

        rg.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener()
        {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId)
            {
                colorBtnVu.setVisibility(View.INVISIBLE);
                colorBtnStrip.setVisibility(View.INVISIBLE);
                colorBtnLamp.setVisibility(View.INVISIBLE);

                switch(checkedId)
                {
                    case R.id.wave: ConnectionManager.getInstance().sendString("4");
                                    break;
                    case R.id.vu1: ConnectionManager.getInstance().sendString("1");
                                    break;
                    case R.id.vu2:colorBtnVu.setVisibility(View.VISIBLE);
                        sendStringWithColor("2", (int)colorBtnVu.getTag());
                        //sendStringWithColor("vu2", (int)colorBtnVu.getTag(),activity);
                        break;
                    case R.id.strip:colorBtnStrip.setVisibility(View.VISIBLE);
                        sendStringWithColor("7", (int)colorBtnStrip.getTag());
                        //sendStringWithColor("7", (int)colorBtnStrip.getTag(), activity);
                        break;
                    case R.id.lamp: colorBtnLamp.setVisibility(View.VISIBLE);
                        sendStringWithColor("6", (int)colorBtnLamp.getTag());
                        //sendStringWithColor("6", (int)colorBtnLamp.getTag(), activity);
                        break;
                }
            }
        });
    }

    public boolean changeConnectionStateTo(final boolean newState, final Activity activity) {
        final ImageButton btnPower = activity.findViewById(R.id.power);
        final RadioGroup rGroup = activity.findViewById(R.id.rGroup);
        final TextView textView = activity.findViewById(R.id.text_home);
        final TextView effects = activity.findViewById(R.id.effects);
        final TextView btnReset = activity.findViewById(R.id.reset);

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < rGroup.getChildCount(); i++) {
                    rGroup.getChildAt(i).setEnabled(newState);
                }
                btnReset.setEnabled(newState);

                if (newState) {
                    btnPower.setBackgroundResource(R.drawable.power_on);
                    textView.setText("Verbunden!");
                    effects.setTextColor(ContextCompat.getColor(activity, R.color.black));
                    ConnectionManager.getInstance().sendString("5");
                    rGroup.clearCheck();
                } else {
                    final Button colorBtnVu = activity.findViewById(R.id.colorBtnVu);
                    final Button colorBtnStrip = activity.findViewById(R.id.colorBtnStrip);
                    final Button colorBtnLamp = activity.findViewById(R.id.colorBtnLamp);

                    btnPower.setBackgroundResource(R.drawable.power_off);
                    textView.setText("Nicht verbunden!");
                    colorBtnVu.setVisibility(View.INVISIBLE);
                    colorBtnStrip.setVisibility(View.INVISIBLE);
                    colorBtnLamp.setVisibility(View.INVISIBLE);

                    effects.setTextColor(ContextCompat.getColor(activity, R.color.colorDisabled));
                }
            }
        });
        return newState;
    }

    public void buildColorPickerDialog(final View pressedButton, final Activity activity){
        ColorPickerDialogBuilder
                .with(activity)
                .setTitle("Wähle eine Farbe aus")
                .initialColor((int)pressedButton.getTag())
                .wheelType(ColorPickerView.WHEEL_TYPE.FLOWER)
                .density(12)
                .noSliders()
                .setPositiveButton("Ok", new ColorPickerClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int selectedColor, Integer[] allColors) {
                        formatColorPickerButton(pressedButton, selectedColor);
                        switch(pressedButton.getId())
                        {
                            case R.id.colorBtnVu:
                                sendStringWithColor("2", selectedColor);
                                //sendStringWithColor("vu2", selectedColor,activity);
                                break;
                            case R.id.colorBtnStrip:
                                sendStringWithColor("7", selectedColor);
                                //sendStringWithColor("7", selectedColor,activity);
                                break;
                            case R.id.colorBtnLamp:
                                sendStringWithColor("6", selectedColor);
                                //sendStringWithColor("6", selectedColor, activity);
                                break;
                        }
                        pressedButton.setTag(selectedColor);
                    }
                })
                .build()
                .show();
    }

    private void formatColorPickerButton(View pressedButton, int selectedColor){
        GradientDrawable gd = new GradientDrawable();
        gd.setColor(selectedColor);
        gd.setCornerRadius(19);
        gd.setStroke(8, 0xff848482);
        pressedButton.setBackground(gd);
        pressedButton.setTag(selectedColor);
    }

    private void sendStringWithColor(String str, int color){
        ConnectionManager.getInstance().sendString(str + Color.red(color) +","+ Color.green(color)+","+ Color.blue(color)+"/");
    }

    private void sendStringWithColor(String str, int color, Activity a){
        ConnectionManager.getInstance().sendString(str + Color.red(color) +","+ Color.green(color)+","+ Color.blue(color)+"/", a);
    }
}
