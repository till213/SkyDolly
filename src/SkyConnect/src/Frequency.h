#ifndef FREQUENCY_H
#define FREQUENCY_H

namespace Frequency
{
    enum Frequency {
        Hz1,
        Hz2,
        Hz5,
        Hz10,
        Hz15,
        Hz20,
        Hz24,
        Hz25,
        Hz30,
        Hz45,
        Hz50,
        Hz60
    };

    inline double toValue(Frequency frequency) {
        switch (frequency) {
        case Hz1:
            return 1.0;
        case Hz2:
            return 2.0;
        case Hz5:
            return 5.0;
        case Hz10:
            return 10.0;
        case Hz15:
            return 15.0;
        case Hz20:
            return 20.0;
        case Hz24:
            return 24.0;
        case Hz25:
            return 25.0;
        case Hz30:
            return 30.0;
        case Hz45:
            return 45.0;
        case Hz50:
            return 50.0;
        case Hz60:
            return 60.0;
        default:
            return 30.0;
        }
    }

    inline Frequency fromValue(double frequency) {
        if (frequency <= 1.0)
            return Hz1;
        else if (frequency <= 1.0)
            return Hz2;
        else if (frequency <= 5.0)
            return Hz5;
        else if (frequency <= 10.0)
            return Hz10;
        else if (frequency <= 15.0)
            return Hz15;
        else if (frequency <= 20.0)
            return Hz20;
        else if (frequency <= 24.0)
            return Hz24;
        else if (frequency <= 25.0)
            return Hz25;
        else if (frequency <= 30.0)
            return Hz30;
        else if (frequency <= 45.0)
            return Hz45;
        else if (frequency <= 50.0)
            return Hz50;
        else if (frequency <= 60.0)
            return Hz60;
        else
            return Hz30;
    }
}

#endif // FREQUENCY_H
