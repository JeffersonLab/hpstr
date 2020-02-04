#include <FunctionMath.h>
#include <TMath.h>

double FunctionMath::ChebyshevFunction(double x, double* p, int order) {
    double total = p[0];
    if(order >= 1) { total += p[1] * x; }
    if(order >= 2) { total += p[2] * (2 * x * x - 1); }
    if(order >= 3) { total += p[3] * (4 * x * x * x - 3 * x); }
    if(order >= 4) { total += p[4] * (8 * x * x * x * x - 8 * x * x + 1); }
    if(order >= 5) { total += p[5] * (16 * x * x * x * x * x - 20 * x * x * x + 5 * x); }
    if(order >= 6) { total += p[6] * (32 * x * x * x * x * x * x - 48 * x * x * x * x + 18 * x * x - 1); }
    if(order >= 7) { total += p[7] * (64 * x * x * x * x * x * x * x - 112 * x * x * x * x * x + 56 * x * x * x - 7 * x); }

    return total;
}

double FunctionMath::Gaussian(double x, double amplitude, double mean, double stddev) {
    return amplitude * 1.0 / (sqrt(2.0 * TMath::Pi() * pow(stddev, 2))) * TMath::Exp(-pow((x - mean), 2) / (2.0 * pow(stddev, 2)));
}

double FunctionMath::CrystalBall(double x, double amplitude, double mean, double stddev, double alpha, double n) {
    // The crystal ball function differs based on the value of x.
    double differentiator = (x - mean) / stddev;
    if(differentiator > -alpha) {
        // Return the functional value.
        return amplitude * exp(-pow(x - mean, 2) / (2 * pow(stddev, 2)));
    } else {
        // Calculate the derived parameters A and B.
        double absAlpha = fabs(alpha);
        double A = calcA(n, absAlpha);
        double B = calcB(n, absAlpha);

        // Return the functional value.
        return amplitude * A * pow(B - ((x - mean) / stddev), -n);
    }
}

double FunctionMath::calcA(double n, double absAlpha) {
    return pow(n / absAlpha, n) * exp(-pow(absAlpha, 2) / 2);
}

double FunctionMath::calcB(double n, double absAlpha) {
    return (n / absAlpha) - absAlpha;
}
