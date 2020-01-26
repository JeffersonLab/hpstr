#ifndef FUNCTIONMATH_H
#define FUNCTIONMATH_H

class FunctionMath {
	public:
		/** Defines a first-order Chebyshev function. */
		static double Order1ChebyshevFunction(double x, double p0, double p1);
		
		/** Defines a third-order Chebyshev function. */
		static double Order3ChebyshevFunction(double x, double p0, double p1, double p2, double p3);
		
		/** Defines a fifth-order Chebyshev function. */
		static double Order5ChebyshevFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5);
		
		/** Defines a seventh-order Chebyshev function. */
		static double Order7ChebyshevFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7);
		
		
		
		/** Defines the first-order exponential background function. */
		static double Order1ExpBackgroundFunction(double x, double p0, double p1);
		
		/** Defines the third-order exponential background function. */
		static double Order3ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3);
		
		/** Defines the fifth-order exponential background function. */
		static double Order5ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5);
		
		/** Defines the seventh-order exponential background function. */
		static double Order7ExpBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7);
		
		
		
		/** Defines the first-order polynomial background function. */
		static double Order1PolBackgroundFunction(double x, double p0, double p1);
		
		/** Defines the third-order polynomial background function. */
		static double Order3PolBackgroundFunction(double x, double p0, double p1, double p2, double p3);
		
		/** Defines the fifth-order polynomial background function. */
		static double Order5PolBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5);
		
		/** Defines the seventh-order polynomial background function. */
		static double Order7PolBackgroundFunction(double x, double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7);
		
		
		
		/** Defines a Gaussian function for signal-fitting. */
		static double Gaussian(double x, double amplitude, double mean, double stddev);
		
		/** Defines a crystal ball function for signal-fitting. */
		static double CrystalBall(double x, double amplitude, double mean, double stddev, double alpha, double n);
	
	private:
		/**
		 * Calculates the third-order background function prior to
		 * the final exponentiation.
		 */
		static double Order3BackgroundFunctionBase(double x, double p0, double p1, double p2, double p3);
		
		/**
		 * Calculates the fifth-order background function prior to
		 * the final exponentiation.
		 */
		static double Order5BackgroundFunctionBase(double x, double p0, double p1, double p2, double p3, double p4, double p5);
		
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