#include "prob.h"
//
//  prob.c
//  
//
//  Created by Mara Helmuth on 9/29/14.
//
//

#include <stdio.h>
#include <math.h>

static	long	randx = 1;

void
srrand(unsigned int x)
{
    randx = x;
}


float rrand()
{
    long i = ((randx = randx * 1103515245 + 12345) >> 16) & 077777;
    return((float)i / 16384. - 1.);
}


double prob(low, mid, high, tight)
double low, mid, high, tight;      /* Returns a value within a range
                                    close to a preferred value

                                    tightness: 0 max away from mid
                                    1 even distribution
                                    2+amount closeness to mid
                                    no negative allowed */
{
    int repeat;
    double range, num, sign;
    repeat = 0;
    do {
        if ((high - mid) > (mid - low))
            range = high - mid;
        else
            range = mid - low;
        if (rrand() > 0.)
            sign = 1.;
        else  sign = -1.;
        num = mid + sign * (pow((rrand() + 1.) * .5, tight) * range);
        if (num < low || num > high)
            repeat++;
        else
            repeat = 0;
    } while (repeat > 0);
    return(num);
}