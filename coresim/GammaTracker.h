#ifndef SIMULATOR_GAMMATRACKER_H
#define SIMULATOR_GAMMATRACKER_H


#include <vector>
#include <string>

class GammaTracker {
public:
    GammaTracker(int n, int t);
    void update(int num);
    int getMinGamma(int size);
    std::string getGammaProfile(int size);
    std::string getErrorsString(int size);

private:
    int n, t;
    int delivered;
    std::vector<int> curNs;
    std::vector<int> curC2s;
    int last_update_num;

};


#endif //SIMULATOR_GAMMATRACKER_H
