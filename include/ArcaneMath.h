#pragma once

class ArcaneMath {
    private:
        float gravity; // gravity
        float yi; // initial y position
        float yf; // final y position
        float vi; // initial velocity
        float vf; // final velocity
        float d; // change in x position
        float theta; // initial launch angle
        float time; // time

        bool gravityKnown;
        bool yiKnown;
        bool yfKnown;
        bool viKnown;
        bool vfKnown;
        bool dKnown;
        bool thetaKnown;
        bool timeKnown;
    public:
        ArcaneMath(float data[8], bool known[8]);
        void solve();
        void print();
};
