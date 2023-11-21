#pragma once

typedef struct TSstruc
{
    float tempUp = 0.0;
    float tempMiddle = 0.0;
    float tempDown = 0.0;
    float hum = 0.0;
    float humDown = 0.0;
    float heater1 = 0.0;
    float heater2 = 0.0;
    float heater3 = 0.0;
}TSstruct;


typedef struct PIRegulation
{
    float Kp;
    float Ki;
    float Ti;
    float sum_err;
    float u;

    void PIControl(float setValue, float refValue)
    {
        float Error=setValue-refValue;
        sum_err=sum_err+Error;
        if (u>1 || u<0)
        {
            sum_err=sum_err-Error;
        }
        u=Kp*Error+Ki/Ti*10*sum_err;

    }
}PIRegulation;

