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
        // Fill the provided array with the current stored values in this order:
        // [0]=gravity, [1]=yi, [2]=yf, [3]=vi, [4]=vf, [5]=d, [6]=theta, [7]=time
        ArcaneMath(float data[8], bool known[8]);
        void writeToArray(float data[8]);

        void solve();
        void print(); // for testing purposes
};
