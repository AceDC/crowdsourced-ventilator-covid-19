#ifndef SIM_PSENS_H
#define SIM_PSENS_H
#include <Arduino.h>
#include "types.h"
#include "i2c_mux.h"

class SimPsens
{
    public:
        SimPsens(QueueHandle_t lungQ, int tca);
        void read();
        float avg;
        float peak;
        float p;
        uint32_t t;
    private:
        QueueHandle_t lungQ;
        Lung_t lung;
        float entries[10];
        float total;
        int idx;
        int tca;
        I2cMux mux;
};

#endif