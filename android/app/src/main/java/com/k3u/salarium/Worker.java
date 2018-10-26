package com.k3u.salarium;

import android.content.Context;
import android.support.annotation.NonNull;
import androidx.work.WorkerParameters;
import retrofit2.Call;
import retrofit2.Response;
import timber.log.Timber;

import java.io.IOException;
import java.text.DateFormat;
import java.util.Date;

public class Worker extends androidx.work.Worker {

    public Worker(@NonNull Context context, @NonNull WorkerParameters workerParams) {
        super(context, workerParams);
    }

    @NonNull
    @Override
    public Result doWork() {

        try {

            long nextAlarm = getInputData().getLong(Constants.NEXT_ALARM, -1);

            if (nextAlarm == -1) {
                Timber.e("Alarm parameter not set");
                return Result.FAILURE;
            }

            Timber.d("Updating remote alarm to: %s", nextAlarm == 0 ? "never" : DateFormat.getDateTimeInstance().format(new Date(nextAlarm)));

            Call call = CloudService.getInstance().updateAlarm(nextAlarm);

            try {
                Response response = call.execute();
                Timber.d("Cloud service return code: %d", response.code());
                return response.isSuccessful() ? Result.SUCCESS : Result.RETRY;
            } catch (IOException e) {
                Timber.e(e, "Exception encountered during cloud service update");
                return Result.RETRY;
            }

        } catch (Exception e) {
            Timber.e(e, "Exception encountered during work execution");
            return Result.FAILURE;
        }

    }

}
