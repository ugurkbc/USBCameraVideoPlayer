#ifndef UTILITY_H
#define UTILITY_H

#include <float.h>
#include <math.h>

enum OKERROR_STATUS
{
    OK = 0,
    ERROR = -1
};


class Utility
{
public:
    static void toFraction(const double decimal, int &numerator_i, int &denominator_i)
    {
        double err = 1.0;
        int denominator = 1;
        double numerator = 0;
        for (int check_denominator = 1; ; check_denominator++)
        {
            double check_numerator = (double)check_denominator * decimal;
            double dummy;
            double check_err = modf(check_numerator, &dummy);
            if (check_err < err)
            {
                err = check_err;
                denominator = check_denominator;
                numerator = check_numerator;
                if (err < FLT_EPSILON)
                    break;
            }
            if (check_denominator == 100)  // limit
                break;
        }
        numerator_i = std::round(numerator);
        denominator_i = denominator;
    }

private:
    Utility() {}
public:
    Utility(Utility const&)               = delete;
    void operator=(Utility const&)  = delete;
};

#endif // UTILITY_H
