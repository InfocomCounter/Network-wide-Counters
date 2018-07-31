#include <sstream>
#include <iostream>
#include "GammaTracker.h"

GammaTracker::GammaTracker(int n_, int t_): n(n_), t(t_), curNs((1 << t)-1, 0), curC2s((1<<t)-1, 0), delivered(0), last_update_num(0) {}

void GammaTracker::update(int num)
{
    delivered++;
    for (int gamma = 1; gamma < (1 << t); gamma++)
    {
        int& curN = curNs[gamma-1];
        if (num > curN && ((num >> (n- t)) - (curN >> (n-t))) <= gamma)
        {
            curN = num;
        }

        int mod = (1 << t) - 1;
        int mid = (num >> (n-t)) & mod;
        int diff = (mid - (curC2s[gamma-1] & mod) + 3*mod + 3) & mod;
        if (diff <= gamma)
        {
//            if (num < last_update_num)
//            {
//                while (true);
//            }
            last_update_num = num;
            curC2s[gamma-1] += diff;
        }

    }

}

int GammaTracker::getMinGamma(int size) {
    for (int i = 0;  i < curNs.size(); i++)
    {
        if (curNs[i] - (curNs[i]%(1<<(n-t))) + (1 << n) > size)
        {
            return i+1;
        }
    }
    return -1;
}

std::string GammaTracker::getGammaProfile(int size) {
    std::stringstream ss;
    ss << "[";
    for (int i = 0;  i < curNs.size(); i++)
    {
        if (curNs[i] - (curNs[i]%(1<<(n-t))) + (1 << n) > size)
        {
            ss << "+";
        }
        else
        {
            ss << "-";
        }
    }
    ss <<"]";
    return ss.str();
}



std::string GammaTracker::getErrorsString(int size) {
    std::stringstream ss;
    bool wasError = false;
    for (int i = 0; i < curC2s.size(); i++)
    {
        int mod = 1 << n;
        int val = curC2s[i] << (n-t);
        int diff = (-val % mod + size % mod + 3*mod) % mod;
        val += diff;

        if (val != size)
        {
            if (!wasError)
            {
                wasError = true;
                ss << "[n = " << n << " " << "t = " << t << " ans = " << size << " delivered = " << delivered << " : ";
            }

            if (val % mod != size % mod)
            {
                std::cerr << "sad(";
            }


            ss << " gamma = " << i+1 << " calculated = " << val << ",";
        }
    }
    if (wasError)
        ss << "]";
    return ss.str();
}



