package com.k3u.salarium;

import android.app.AlarmManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import androidx.work.*;

public class AlarmReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {

        long nextAlarm = 0;
        AlarmManager alarmManager = context.getSystemService(AlarmManager.class);
        if (alarmManager != null) {
            AlarmManager.AlarmClockInfo alarmClockInfo = alarmManager.getNextAlarmClock();
            if (alarmClockInfo != null) {
                nextAlarm = alarmClockInfo.getTriggerTime();
            }
        }

        Constraints constraints =
                new Constraints.Builder()
                        .setRequiredNetworkType(NetworkType.CONNECTED)
                        .build();

        Data data =
                new Data.Builder()
                        .putLong(Constants.NEXT_ALARM, nextAlarm)
                        .build();

        OneTimeWorkRequest workRequest =
                new OneTimeWorkRequest.Builder(Worker.class)
                        .setConstraints(constraints)
                        .setInputData(data)
                        .build();

        WorkManager
                .getInstance()
                .beginUniqueWork(Constants.WORK_TAG, ExistingWorkPolicy.REPLACE, workRequest)
                .enqueue();

    }

}
