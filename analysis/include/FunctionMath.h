#ifndef FUNCTIONMATH_H
#define FUNCTIONMATH_H

class FunctionMath {
    public:
        /** Defines a Chebyshev polynomial function. */
        static double ChebyshevFunction(double x, double* p, int order);
        
        /** Define a Legendre polynomial function. */
        static double LegendreFunction(double x, double* p, int order);

        /** Defines a Gaussian function for signal-fitting. */
        static double Gaussian(double x, double amplitude, double mean, double stddev);

        /** Defines a crystal ball function for signal-fitting. */
        static double CrystalBall(double x, double amplitude, double mean, double stddev, double alpha, double n);
	/** Defines a Landau function for signal-fitting. */
	static double Landau(double x, double mu, double sigma, double amplitude, double shift);

	static double HeavisideFunction(double x, double c);

    private:
        /**
         * Calculates a portion of the crystal ball function.
         */
        static double calcA(double n, double absAlpha);

        /**
         * Calculates a portion of the crystal ball function.
         */
        static double calcB(double n, double absAlpha);
};

#endif
