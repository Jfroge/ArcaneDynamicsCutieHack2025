#include "../include/ArcaneMath.h"
#include <cmath>
#include <iostream>

const float PI = 3.14159265358979323846f;

// Convert degrees to radians
float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

// Convert radians to degrees
float radiansToDegrees(float radians) {
    return radians * 180.0f / PI;
}

ArcaneMath::ArcaneMath(float data[8], bool known[8]) {
    // initialize members to safe defaults
    gravity = yi = yf = vi = vf = d = theta = time = 0.0f;
    gravityKnown = yiKnown = yfKnown = viKnown = vfKnown = dKnown = thetaKnown = timeKnown = false;

    // Copy provided inputs (guarded)
    if (known != nullptr && data != nullptr) {
        gravity   = data[0]; gravityKnown = known[0];
        yi        = data[1]; yiKnown      = known[1];
        yf        = data[2]; yfKnown      = known[2];
        vi        = data[3]; viKnown      = known[3];
        vf        = data[4]; vfKnown      = known[4];
        d         = data[5]; dKnown       = known[5];
        theta     = data[6]; thetaKnown   = known[6];
        time      = data[7]; timeKnown    = known[7];
    }

    // Validate known flags correspond to finite data; if not, clear the flag and warn
    auto validate = [&](const char *name, float value, bool &flag) {
        if (flag && !std::isfinite(value)) {
            std::cerr << "Warning: input '" << name << "' marked known but value is not finite. Ignoring.\n";
            flag = false;
        }
    };

    validate("gravity", gravity, gravityKnown);
    validate("yi", yi, yiKnown);
    validate("yf", yf, yfKnown);
    validate("vi", vi, viKnown);
    validate("vf", vf, vfKnown);
    validate("d", d, dKnown);
    validate("theta", theta, thetaKnown);
    validate("time", time, timeKnown);

    // If theta was provided in degrees, convert to radians now for internal use
    if (thetaKnown) {
        theta = degreesToRadians(theta);
    }
}

void ArcaneMath::solve() {
    const float EPS = 1e-6f;
    bool updated;
    int iterations = 0;
    const int MAX_ITERATIONS = 100;

    // theta conversion is handled in the constructor; nothing to do here

    do {
        updated = false;
        iterations++;

        // --- Horizontal motion ---

        // Solve time from horizontal motion: d = vi * cos(theta) * t
        if (!timeKnown && dKnown && viKnown && thetaKnown) {
            float cosTheta = std::cos(theta);
            if (std::abs(cosTheta) > EPS) {
                time = d / (vi * cosTheta);
                timeKnown = true;
                updated = true;
            }
        }

        // Solve horizontal distance: d = vi * cos(theta) * t
        if (!dKnown && viKnown && thetaKnown && timeKnown) {
            d = vi * std::cos(theta) * time;
            dKnown = true;
            updated = true;
        }

        // Solve initial velocity from horizontal motion: vi = d / (cos(theta) * t)
        if (!viKnown && dKnown && thetaKnown && timeKnown) {
            float cosTheta = std::cos(theta);
            if (std::abs(cosTheta) > EPS && std::abs(time) > EPS) {
                vi = d / (cosTheta * time);
                viKnown = true;
                updated = true;
            }
        }

        // Solve launch angle from horizontal motion: theta = acos(d / (vi * t))
        if (!thetaKnown && dKnown && viKnown && timeKnown) {
            float denom = vi * time;
            if (std::abs(denom) > EPS) {
                float cosTheta = d / denom;
                if (cosTheta >= -1.0f && cosTheta <= 1.0f) {
                    theta = std::acos(cosTheta);
                    thetaKnown = true;
                    updated = true;
                }
            }
        }

        // --- Vertical motion ---

        // Assume yi = 0 if not known (projectile starts at ground level)
        if (!yiKnown && !yfKnown && viKnown && timeKnown && gravityKnown && thetaKnown) {
            yi = 0.0f;
            yiKnown = true;
            updated = true;
        }

        // Solve final vertical position: yf = yi + vi*sin(theta)*t - 0.5*g*t^2
        if (!yfKnown && yiKnown && viKnown && timeKnown && gravityKnown && thetaKnown) {
            yf = yi + vi * std::sin(theta) * time - 0.5f * gravity * time * time;
            yfKnown = true;
            updated = true;
        }

        // Solve initial vertical position: yi = yf - vi*sin(theta)*t + 0.5*g*t^2
        if (!yiKnown && yfKnown && viKnown && timeKnown && gravityKnown && thetaKnown) {
            yi = yf - vi * std::sin(theta) * time + 0.5f * gravity * time * time;
            yiKnown = true;
            updated = true;
        }

        // Solve initial velocity from vertical motion: vi = (yf - yi + 0.5*g*t^2) / (sin(theta)*t)
        if (!viKnown && yiKnown && yfKnown && timeKnown && gravityKnown && thetaKnown) {
            float sinTheta = std::sin(theta);
            if (std::abs(sinTheta) > EPS && std::abs(time) > EPS) {
                vi = (yf - yi + 0.5f * gravity * time * time) / (sinTheta * time);
                viKnown = true;
                updated = true;
            }
        }

        // Solve launch angle from vertical motion: theta = asin((yf - yi + 0.5*g*t^2) / (vi*t))
        if (!thetaKnown && yiKnown && yfKnown && viKnown && timeKnown && gravityKnown) {
            float denom = vi * time;
            if (std::abs(denom) > EPS) {
                float sinTheta = (yf - yi + 0.5f * gravity * time * time) / denom;
                if (sinTheta >= -1.0f && sinTheta <= 1.0f) {
                    theta = std::asin(sinTheta);
                    thetaKnown = true;
                    updated = true;
                }
            }
        }

        // Solve time from vertical motion (quadratic): yf = yi + vi*sin(theta)*t - 0.5*g*t^2
        if (!timeKnown && yiKnown && yfKnown && viKnown && gravityKnown && thetaKnown) {
            float a = -0.5f * gravity;
            float b = vi * std::sin(theta);
            float c = yi - yf;

            float disc = b*b - 4*a*c;
            if (disc >= 0 && std::abs(a) > EPS) {
                float t1 = (-b + std::sqrt(disc)) / (2*a);
                float t2 = (-b - std::sqrt(disc)) / (2*a);
                time = (t1 > EPS) ? t1 : t2;
                timeKnown = true;
                updated = true;
            }
        }

        // --- Final velocity magnitude ---

        // Solve final velocity magnitude: vf = sqrt(vx^2 + vy^2)
        if (!vfKnown && viKnown && gravityKnown && timeKnown && thetaKnown) {
            float vx = vi * std::cos(theta);          // horizontal velocity (constant)
            float vy = vi * std::sin(theta) - gravity * time; // vertical velocity
            vf = std::sqrt(vx*vx + vy*vy);
            vfKnown = true;
            updated = true;
        }

    } while(updated && iterations < MAX_ITERATIONS);

    // Convert theta back to degrees for output
    if (thetaKnown) {
        theta = radiansToDegrees(theta);
    }
}



void ArcaneMath::print() {
        std::cout << "g = " << gravity << ", yi = " << yi << ", yf = " << yf
                  << ", vi = " << vi << ", vf = " << vf << ", d = " << d
                  << ", theta = " << theta << ", t = " << time << std::endl;
}