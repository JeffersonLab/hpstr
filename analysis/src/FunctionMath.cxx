#include <FunctionMath.h>
#include <TMath.h>

double FunctionMath::Order1ChebyshevFunction(double x, double p0, double p1) {
	// Calculate the terms for each order.
    double t0 = p0;
    double t1 = p1 * x;
	
	// Return the sum.
	double pol = t0 + t1;
}

double FunctionMath::Order3ChebyshevFunction(double x, double p0, double p1, double p2, double p3) {
	// Higher-order Chebyshev polynomials contain the lower order
	// polynomials entirely.
	double o1c = Order1ChebyshevFunction(x, p0, p1);
	
	// Calculate the additional terms.
    double t2 = p2 * (2 * x * x - 1);
    double t3 = p3 * (4 * x * x * x - 3 * x);
	
	// Return the sum.
	return o1c + t2 + t3;
}

double FunctionMath::Order5ChebyshevFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5) {
	// Higher-order Chebyshev polynomials contain the lower order
	// polynomials entirely.
	double o3c = Order3ChebyshevFunction(x, p0, p1, p2, p3);
	
	// Calculate the additional terms.
    double t4 = p4 * (8 * x * x * x * x - 8 * x * x + 1);
    double t5 = p5 * (16 * x * x * x * x * x - 20 * x * x * x + 5 * x);
	
	// Return the sum.
	return o3c + t4 + t5;
}

double FunctionMath::Order7ChebyshevFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7) {
	// Higher-order Chebyshev polynomials contain the lower order
	// polynomials entirely.
	double o5c = Order5ChebyshevFunction(x, p0, p1, p2, p3, p4, p5);
	
	// Calculate the additional terms.
    double t6 = p6 * (32 * x * x * x * x * x * x - 48 * x * x * x * x + 18 * x * x - 1);
    double t7 = p7 * (64 * x * x * x * x * x * x * x - 112 * x * x * x * x * x + 56 * x * x * x - 7 * x);
	
	// Return the sum.
	return o5c + t6 + t7;
}



double FunctionMath::Order1ExpBackgroundFunction(double x, double p0, double p1) {
	double pol = FunctionMath::Order1ChebyshevFunction(x, p0, p1);
	return TMath::Power(10, pol);
}

double FunctionMath::Order3ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3) {
	double pol = FunctionMath::Order3ChebyshevFunction(x, p0, p1, p2, p3);
	return TMath::Power(10, pol);
}

double FunctionMath::Order5ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5) {
	double pol = FunctionMath::Order5ChebyshevFunction(x, p0, p1, p2, p3, p4, p5);
	return TMath::Power(10, pol);
}

double FunctionMath::Order7ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7) {
	double pol = FunctionMath::Order7ChebyshevFunction(x, p0, p1, p2, p3, p4, p5, p6, p7);
	return TMath::Power(10, pol);
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
