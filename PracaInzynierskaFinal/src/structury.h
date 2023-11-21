#pragma once

typedef struct TSstruc
{
    float tempUp = 0.0;
    float tempMiddle = 0.0;
    float tempDown = 0.0;
    float hum = 0.0;
    float humDown = 0.0;
    float heater = 0.0;

}TSstruct;


typedef struct PIRegulation
{
    float Kp;
    float Ki;
    float Ti;
    float sum_err;
    float u;
    float PIController(float, float);
}PIRegulation;

