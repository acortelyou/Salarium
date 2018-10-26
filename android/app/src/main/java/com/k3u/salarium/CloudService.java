package com.k3u.salarium;

import retrofit2.Call;
import retrofit2.Retrofit;
import retrofit2.http.Field;
import retrofit2.http.FormUrlEncoded;
import retrofit2.http.PUT;

class CloudService {

    public interface API {

        @FormUrlEncoded
        @PUT("alarm")
        Call<Void> updateAlarm(@Field("next") long next);

    }

    private static CloudService.API instance;

    static CloudService.API getInstance() {
        if (instance == null) {
            Retrofit retrofit = new Retrofit.Builder()
                    .baseUrl(Constants.SERVICE_BASE_URL)
                    .build();
            instance = retrofit.create(CloudService.API.class);
        }
        return instance;
    }

}
