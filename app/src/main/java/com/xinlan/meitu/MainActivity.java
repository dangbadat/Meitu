package com.xinlan.meitu;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;

import java.lang.ref.WeakReference;

public class MainActivity extends AppCompatActivity implements View.OnClickListener, SeekBar.OnSeekBarChangeListener {
    private ImageView mImage;
    private Button mBtn;
    private SeekBar mSmoothBar;
    private SeekBar mWhiteBar;

    private Bitmap bit;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mImage = (ImageView) findViewById(R.id.image);
        mBtn = (Button) findViewById(R.id.btn);

        mSmoothBar = (SeekBar) findViewById(R.id.seek_bar_smooth);
        mWhiteBar = (SeekBar) findViewById(R.id.seek_bar_white);
        mBtn.setOnClickListener(this);

        mSmoothBar.setOnSeekBarChangeListener(this);
        mWhiteBar.setOnSeekBarChangeListener(this);

        bit = BitmapFactory.decodeResource(getResources(), R.drawable.mopi6);
        mImage.setImageBitmap(bit);

        System.out.println("on create bit Bitmap  ---> "+bit.hashCode());
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        int whiteVal = mWhiteBar.getProgress();
        int smoothVal = mSmoothBar.getProgress();

        WeakReference<Bitmap> handleBitmap = new WeakReference<Bitmap>(bit.copy(bit.getConfig(),true));
        System.out.println("bit Bitmap  ---> "+bit.hashCode());
        System.out.println("WeakReference ---> "+handleBitmap.get().hashCode());

        Tools.handleSmoothAndWhiteSkin(handleBitmap.get(), smoothVal, whiteVal);
        mImage.setImageBitmap(handleBitmap.get());

//        Bitmap handleBit = BitmapFactory.decodeResource(getResources(), R.drawable.mopi2);
//        Tools.handleSmoothAndWhiteSkin(handleBit, smoothVal, whiteVal);
//        mImage.setImageBitmap(handleBit);
    }

    @Override
    public void onClick(View view) {
//        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),R.drawable.mopi3);
//        Bitmap dealBit = Bitmap.createBitmap(bitmap);
//        //Tools.processImage(bitmap);
//        Tools.handleBeautify(dealBit,100);
//        mImage.setImageBitmap(dealBit);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

}//end class
