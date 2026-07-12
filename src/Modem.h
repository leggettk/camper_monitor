#pragma once

#include <TinyGsmClient.h>

class Modem
{
public:
    bool begin();
};

extern Modem cellular;
extern TinyGsm gsm;