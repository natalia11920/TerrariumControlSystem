#pragma once

typedef struct TSstruct
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
    float w1;
    float w2;
    float w3;
    float sum_err = 0.0f;
    float u = 0.5;
    float u1 = 0.5;
    float u2 = 0.5;
    float u3 = 0.5;

    void PIControl(float, float);

}PIRegulation;

void PIRegulation::PIControl(float setValue, float refValue)
{
        float uMax=1.0, uMin=0.0;
        float Error=setValue-refValue;
        sum_err=sum_err+Error;
        
        u=Kp*Error+Kp/Ti*30*sum_err;

        u1=w1*u+0.5;
        u2=w2*u+0.5;
        u3=w3*u+0.5;     

        if ( u1>uMax || u2>uMax || u3>uMax ) 
        {
            sum_err=sum_err-Error;
        }
        if ( u1<uMin || u2<uMin || u3<uMin) 
        {
            sum_err=sum_err-Error;
        }

        if (u1<uMin)
        {
            u1=uMin;
        }
        if (u1>uMax)
        {
            u1=uMax;
        }
        
        if (u2<uMin)
        {
            u2=uMin;
        }
        if (u2>uMax)
        {
            u2=uMax;
        }

        if (u3<uMin)
        {
            u3=uMin;
        }
        if (u3>uMax)
        {
            u3=uMax;
        }
        

}





