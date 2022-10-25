#ifndef FUNCTIONMATH_H
#define FUNCTIONMATH_H

class FunctionMath {
    public:
        /**
         * @brief Defines a Chebyshev polynomial function.
         * 
         * @param x 
         * @param p 
         * @param order 
         * @return double 
         */
        static double ChebyshevFunction(double x, double* p, int order);

        /**
         * @brief Define a Legendre polynomial function.
         * 
         * @param x 
         * @param p 
         * @param order 
         * @return double 
         */
        static double LegendreFunction(double x, double* p, int order);

        /**
         * @brief Defines a Gaussian function for signal-fitting.
         * 
         * @param x 
         * @param amplitude 
         * @param mean 
         * @param stddev 
         * @return double 
         */
        static double Gaussian(double x, double amplitude, double mean, double stddev);

        /**
         * @brief Defines a crystal ball function for signal-fitting.
         * 
         * @param x 
         * @param amplitude 
         * @param mean 
         * @param stddev 
         * @param alpha 
         * @param n 
         * @return double 
         */
        static double CrystalBall(double x, double amplitude, double mean,
                                  double stddev, double alpha, double n);

    private:
        /**
         * @brief Calculates a portion of the crystal ball function.
         * 
         * @param n 
         * @param absAlpha 
         * @return double 
         */
        static double calcA(double n, double absAlpha);

        /**
         * @brief Calculates a portion of the crystal ball function.
         * 
         * @param n 
         * @param absAlpha 
         * @return double 
         */
        static double calcB(double n, double absAlpha);
};

#endif
