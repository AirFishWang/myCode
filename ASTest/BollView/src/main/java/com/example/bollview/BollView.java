package com.example.bollview;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

/**
 * Created by Administrator on 2017/9/22.
 */

public class BollView extends View {
    public BollView(Context context)
    {
     super(context);
    }

    public BollView(Context context, AttributeSet set)
    {
        super(context, set);
    }

    public float currentX = 40;
    public float currentY = 50;
    Paint p = new Paint();

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        p.setColor(Color.RED);
        canvas.drawCircle(currentX, currentY, 25, p);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        currentX = event.getX();
        currentY = event.getY();

        invalidate();
        return true;
    }

}
