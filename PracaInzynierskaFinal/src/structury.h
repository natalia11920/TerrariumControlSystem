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
    float Ti;
    float sum_err = 0.0f;
    float u = 0.0f;

    void PIControl(float setValue, float refValue)
    {
        float Error=setValue-refValue;
        sum_err=sum_err+Error;
        u=Kp*Error+Kp/Ti*30*sum_err;
        if (u>0.5) 
        {
            sum_err=sum_err-Error;
            u=0.5; 
        }
        if (u<-0.5) 
        {
            sum_err=sum_err-Error;
            u=-0.5; 
        }

    }

}PIRegulation;

