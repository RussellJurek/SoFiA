/// ____________________________________________________________________ ///
///                                                                      ///
/// busyfit (BusyFit.cpp) - Busy Function fitting programme              ///
/// Copyright (C) 2013 Tobias Westmeier                                  ///
/// ____________________________________________________________________ ///
///                                                                      ///
/// Address:  Tobias Westmeier                                           ///
///           ICRAR M468                                                 ///
///           The University of Western Australia                        ///
///           35 Stirling Highway                                        ///
///           Crawley WA 6009                                            ///
///           Australia                                                  ///
///                                                                      ///
/// E-mail:   tobias.westmeier@uwa.edu.au                                ///
/// ____________________________________________________________________ ///
///                                                                      ///
/// This program is free software: you can redistribute it and/or modify ///
/// it under the terms of the GNU General Public License as published by ///
/// the Free Software Foundation, either version 3 of the License, or    ///
/// (at your option) any later version.                                  ///
///                                                                      ///
/// This program is distributed in the hope that it will be useful,      ///
/// but WITHOUT ANY WARRANTY; without even the implied warranty of       ///
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         ///
/// GNU General Public License for more details.                         ///
///                                                                      ///
/// You should have received a copy of the GNU General Public License    ///
/// along with this program. If not, see http://www.gnu.org/licenses/.   ///
/// ____________________________________________________________________ ///
///                                                                      ///

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>      // Needed to invoke gnuplot via system().

#include "helperFunctions.h"
#include "BusyFit.h"


BusyFit::BusyFit()
{
    spectrum.nChannels = 0;
    spectrum.values    = 0;
    spectrum.sigma     = 0;
    
    a   = 0.0;
    b1  = 0.0;
    b2  = 0.0;
    c   = 0.0;
    xe0 = 0.0;
    xp0 = 0.0;
    w   = 0.0;
    
    init_a   = 0.0;
    init_b1  = 0.0;
    init_b2  = 0.0;
    init_c   = 0.0;
    init_xe0 = 0.0;
    init_xp0 = 0.0;
    init_w   = 0.0;
    
    err_a   = 0.0;
    err_b1  = 0.0;
    err_b2  = 0.0;
    err_c   = 0.0;
    err_xe0 = 0.0;
    err_xp0 = 0.0;
    err_w   = 0.0;
    
    chiSquare = 0.0;
    order     = 2;
    
    flagNoPlot = false;
    flagRelax  = false;
    
    for(int i = 0; i < BUSYFIT_FREE_PARAM; i++) spectrum.mask[i] = true;
    
    return;
}


int BusyFit::setup(size_t n, double *newData, double *newSigma, int newOrder, bool noPlot, bool relax)
{
    if(n < 10)
    {
        std::cerr << "Error (BusyFit): Not enough spectral channels to fit." << std::endl;
        return 1;
    }
    
    spectrum.nChannels = n;
    spectrum.values    = newData;
    spectrum.sigma     = newSigma;
    
    flagNoPlot = noPlot;
    flagRelax  = relax;
    
    if(newOrder != 2 and newOrder != 4)
    {
        std::cerr << "Warning (BusyFit): Unsupported polynomial order (" << newOrder << "); using 2 instead." << std::endl;
        order = 2;
    }
    else
    {
        order = newOrder;
    }
    
    return 0;
}

int BusyFit::setFreeParameters(bool newMask[BUSYFIT_FREE_PARAM])
{
    for(int i = 0; i < BUSYFIT_FREE_PARAM; i++) spectrum.mask[i] = newMask[i];
    
    return 0;
}



int BusyFit::fit()
{
    if(spectrum.nChannels == 0 or spectrum.values == 0 or spectrum.sigma == 0)
    {
        std::cerr << "Error (BusyFit): No valid data found." << std::endl;
        return 2;
    }
    
    // Define initial estimates:
    init_a   = 0.0;
    init_b1  = 0.5;
    init_b2  = 0.5;
    init_c   = 0.01;
    init_xe0 = 0.0;
    init_xp0 = 0.0;
    init_w   = 0.0;
    
    size_t posMax   = 0.0;
    
    // Calculate maximum to use as a:
    for(size_t i = 0; i < spectrum.nChannels; i++)
    {
        if(spectrum.values[i] > init_a)
        {
            init_a = spectrum.values[i];
            posMax = i;
        }
    }
    
    init_a /= 1.5;      // Make a a little bit smaller, as it actually is the value at the profile centre, not the peak.
    
    // Find flanks to determine position and width:
    size_t leftFlank  = 0;
    size_t rightFlank = 0;
    
    size_t currentPos = posMax;
    
    while(currentPos > 0 and spectrum.values[currentPos] > 0.2 * init_a)
    {
        currentPos--;
    }
    
    leftFlank = currentPos;
    
    currentPos = posMax;
    
    while(currentPos < spectrum.nChannels - 1 and spectrum.values[currentPos] > 0.2 * init_a)
    {
        currentPos++;
    }
    
    rightFlank = currentPos;
    
    if(leftFlank >= 0 and rightFlank - leftFlank > 2)
    {
        init_xe0 = static_cast<double>(rightFlank + leftFlank) / 2.0;
        init_xp0 = init_xe0;
        init_w   = static_cast<double>(rightFlank - leftFlank - 2) / 2.0;
    }
    else
    {
        std::cerr << "Warning (BusyFit): Failed to find line flanks." << std::endl;
        std::cerr << "                   Calculating moments instead." << std::endl;
        
        // Calculate first moment to use as xe0 and xp0:
        double sum = 0.0;
        
        for(size_t i = 0; i < spectrum.nChannels; i++)
        {
            if(spectrum.values[i] > 0.1 * init_a)
            {
                init_xe0 += spectrum.values[i] * static_cast<double>(i);
                sum += spectrum.values[i];
            }
        }
        
        init_xe0 /= sum;
        init_xp0 = init_xe0;
        
        // Calculate second moment to use as w:
        sum = 0.0;
        
        for(size_t i = 0; i < spectrum.nChannels; i++)
        {
            if(spectrum.values[i] > 0.1 * init_a)
            {
                init_w += spectrum.values[i] * (static_cast<double>(i) - init_xe0) * (static_cast<double>(i) - init_xe0);
                sum += spectrum.values[i];
            }
        }
        
        init_w = sqrt(init_w / sum);
    }
    
    // Print initial estimates:
    //std::cout << "\nUsing the following initial estimates:\n\n";
    
    //std::cout << "A   =\t" << init_a   << "\n";
    //std::cout << "B₁  =\t" << init_b1  << "\n";
    //std::cout << "B₂  =\t" << init_b2  << "\n";
    //std::cout << "C   =\t" << init_c   << "\n";
    //std::cout << "XE₀ =\t" << init_xe0 << "\n";
    //std::cout << "XP₀ =\t" << init_xp0 << "\n";
    //std::cout << "W   =\t" << init_w   << std::endl;
    
    // Copy initial estimates into parameter variables:
    a   = init_a;
    b1  = init_b1;
    b2  = init_b2;
    c   = init_c;
    xe0 = init_xe0;
    xp0 = init_xp0;
    w   = init_w;
    
    return fitWithEstimates();
}

int BusyFit::fit(double new_a, double new_b1, double new_b2, double new_c, double new_xe0, double new_xp0, double new_w)
{
    if(spectrum.nChannels == 0 or spectrum.values == 0 or spectrum.sigma == 0)
    {
        std::cerr << "Error (BusyFit): No valid data found." << std::endl;
        return 2;
    }
    
    a   = init_a   = new_a;
    b1  = init_b1  = new_b1;
    b2  = init_b2  = new_b2;
    c   = init_c   = new_c;
    xe0 = init_xe0 = new_xe0;
    xp0 = init_xp0 = new_xp0;
    w   = init_w   = new_w;
    
    return fitWithEstimates();
}



int BusyFit::fitWithEstimates()
{
    int status = 0;
    
    // Initial fit:
    status = LMSolver();
    
    // Polynomial negative or offset?
    if(flagRelax == false and (c < 0.0 or fabs(xe0 - xp0) > 2.0 * init_w))
    {
        a   = init_a;
        b1  = init_b1;
        b2  = init_b2;
        c   = 0.0;
        xe0 = init_xe0;
        xp0 = 0.0;
        w   = init_w;
        
        spectrum.mask[3] = false;
        spectrum.mask[5] = false;
        
        std::cerr << "Warning (BusyFit): Repeating fit without polynomial component (C = 0)." << std::endl;
        
        // Fit again with c and xp0 fixed to 0:
        status = LMSolver();
    }
    
    // Plot and print results:
    if(flagNoPlot == false) plotResult();
    printResult();
    
    if(status == 0)
    {
        // Success:
        if(a <= 0.0 or b1 <= 0.0 or b2 <= 0.0 or w <= 0.0 or c < 0)
        {
            // Warn that some parameters are negative:
            std::cerr << "Warning (BusyFit): Fit successful, but some parameters zero or negative.\n" << std::endl;
            return 1;
        }
    }
    else
    {
        // Failure:
        std::cerr << "Error (BusyFit): Failed to fit spectrum." << std::endl;
        return 2;
    }
    
    return 0;
}



int BusyFit::plotResult()
{
    std::ofstream outputFileSpectrum("busyfit_output_spectrum.txt");
    std::ofstream outputFileFit("busyfit_output_fit.txt");
    
    if(outputFileSpectrum.is_open() and outputFileFit.is_open())
    {
        for(size_t i = 0; i < spectrum.nChannels; i++)
        {
            double ii = static_cast<double>(i);
            outputFileSpectrum << i << '\t' << spectrum.values[i] << '\t' << spectrum.values[i] - B(ii, order) << '\n';
        }
        
        outputFileSpectrum.close();
        
        for(double x = 0.0; x < static_cast<double>(spectrum.nChannels - 1); x += static_cast<double>(spectrum.nChannels) / 1000.0)
        {
            outputFileFit << x << '\t' << B(x, order)  << '\n';
        }
        
        outputFileFit.close();
    }
    else
    {
        std::cerr << "Error (BusyFit): Unable to write output files." << std::endl;;
        return 1;
    }
    
    std::string gnuplotCommand = "echo \'unset key; set grid; set xlabel \"Spectral Channel\"; set ylabel \"Brightness\"; plot \"busyfit_output_spectrum.txt\" using 1:3 with points linecolor rgb \"#4060C0\", \"busyfit_output_spectrum.txt\" using 1:2 with histeps linecolor rgb \"#808080\", \"busyfit_output_fit.txt\" using 1:2 with lines linecolor rgb \"#FF0000\"\' | gnuplot -persist";
    
    return system(gnuplotCommand.c_str());
}



int BusyFit::printResult()
{
    // Print results on screen
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(4) << "chisq/dof = " << chiSquare << "\n";
    
    std::cout << std::setprecision(4) << "\nA   =\t" << a   << "\t   ±\t" << err_a;
    if(spectrum.mask[0] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << 100.0 * a / init_a << "%)";
    
    std::cout << std::setprecision(4) << "\nB₁  =\t" << b1  << "\t   ±\t" << err_b1;
    if(spectrum.mask[1] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << 100.0 * b1 / init_b1 << "%)";
    
    std::cout << std::setprecision(4) << "\nB₂  =\t" << b2  << "\t   ±\t" << err_b2;
    if(spectrum.mask[2] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << 100.0 * b2 / init_b2 << "%)";
    
    std::cout << std::setprecision(4) << "\nC   =\t" << c   << "\t   ±\t" << err_c;
    if(spectrum.mask[3] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << 100.0 * c / init_c << "%)";
    
    std::cout << std::setprecision(4) << "\nXE₀ =\t" << xe0 << "\t   ±\t" << err_xe0;
    if(spectrum.mask[4] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << xe0 - init_xe0 << ")";
    
    std::cout << std::setprecision(4) << "\nXP₀ =\t" << xp0 << "\t   ±\t" << err_xp0;
    if(spectrum.mask[5] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << xp0 - init_xp0 << ")";
    
    std::cout << std::setprecision(4) << "\nW   =\t" << w   << "\t   ±\t" << err_w;
    if(spectrum.mask[6] == false) std::cout << "\t(FIXED)";
    else std::cout << std::setprecision(1) << "\t(" << 100.0 * w / init_w << "%)";
    
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(4) << '\n' << std::endl;
    
    return 0;
}



int BusyFit::getResult(double *parValues, double *parUncert, double &chi2)
{
    parValues[0] = a;
    parValues[1] = b1;
    parValues[2] = b2;
    parValues[3] = c;
    parValues[4] = xe0;
    parValues[5] = xp0;
    parValues[6] = w;
    
    parUncert[0] = err_a;
    parUncert[1] = err_b1;
    parUncert[2] = err_b2;
    parUncert[3] = err_c;
    parUncert[4] = err_xe0;
    parUncert[5] = err_xp0;
    parUncert[6] = err_w;
    
    chi2         = chiSquare;
    
    return 0;
}



int BusyFit::getParameters(double &posX, double &w50, double &w20, double &Fpeak, double &Fint)
{
    if(a == 0) return 1;
    
    double limitLo = 0.0;
    double limitHi = static_cast<double>(spectrum.nChannels);
    double step  = (limitHi - limitLo) / 1.0e+5;
    
    Fpeak = 0.0;
    
    // Find maximum:
    for(double x = limitLo; x < limitHi; x += step)
    {
        if(B(x, order) > Fpeak) Fpeak = B(x, order);
    }
    
    // Determine w50 and w20:
    double current_x = limitLo;
    
    while(current_x < limitHi and B(current_x, order) < Fpeak / 5.0)
    {
        current_x += step;
    }
    
    double w20Lo = current_x;
    
    while(current_x < limitHi and B(current_x, order) < Fpeak / 2.0)
    {
        current_x += step;
    }
    
    double w50Lo = current_x;
    
    current_x = limitHi;
    
    while(current_x > w20Lo and B(current_x, order) < Fpeak / 5.0)
    {
        current_x -= step;
    }
    
    w20 = current_x - w20Lo;
    
    while(current_x > w50Lo and B(current_x, order) < Fpeak / 2.0)
    {
        current_x -= step;
    }
    
    w50 = current_x - w50Lo;
    
    // Determine integral and centroid:
    Fint = 0.0;
    posX = 0.0;
    
    for(double x = limitLo; x < limitHi; x += step)
    {
        Fint += B(x, order);
        posX += B(x, order) * x;
    }    
    
    posX /= Fint;
    Fint *= step;
    
    //std::cout << "Pos   = " << posX << "\t[chan]\n" << "w₅₀   = " << w50 << "\t[chan]\nw₂₀   = " << w20 << "\t[chan]\nFpeak = " << Fpeak << "\t[flux]\nFint  = " << Fint << "\t[flux × chan]\n" << std::endl;
    
    return 0;
}



// Actual fitting routine

int BusyFit::LMSolver()
{
    int                status;
    unsigned int       iter    = 0;              // Iteration
    const unsigned int iterMax = 10000;          // Maximum number of iterations
    double             x_init[BUSYFIT_FREE_PARAM] = {a, b1, b2, c, xe0, xp0, w};         // Initial estimates of free parameters
    
    // Setup of GSL solver
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver            *s;
    gsl_matrix                        *covar = gsl_matrix_alloc(BUSYFIT_FREE_PARAM, BUSYFIT_FREE_PARAM);
    gsl_multifit_function_fdf          f;
    gsl_vector_view                    x     = gsl_vector_view_array(x_init, BUSYFIT_FREE_PARAM);
    const gsl_rng_type                *type;
    gsl_rng                           *r;
    
    gsl_rng_env_setup();
    
    type     = gsl_rng_default;
    r        = gsl_rng_alloc(type);
    
    if(order == 4)
    {
        f.f      = &expb_f2;
        f.df     = &expb_df2;
        f.fdf    = &expb_fdf2;
    }
    else
    {
        f.f      = &expb_f;
        f.df     = &expb_df;
        f.fdf    = &expb_fdf;
    }
    
    f.n      = spectrum.nChannels;
    f.p      = BUSYFIT_FREE_PARAM;
    f.params = &spectrum;
    
    T        = gsl_multifit_fdfsolver_lmsder;
    s        = gsl_multifit_fdfsolver_alloc(T, spectrum.nChannels, BUSYFIT_FREE_PARAM);
    gsl_multifit_fdfsolver_set(s, &f, &x.vector);
    
    if(order == 4) std::cout << "\nFitting \"Busy Function\" B₂(x) to spectrum:\nB₂(x) = (A / 4) × [erf(B1 (W + X − XE₀)) + 1]\n        × [erf(B2 (W − X + XE₀)) + 1] × [C (X − XP₀)⁴ + 1]" << std::endl;
    else std::cout << "\nFitting \"Busy Function\" B(x) to spectrum:\nB(x) = (A / 4) × [erf(B1 (W + X − XE₀)) + 1]\n       × [erf(B2 (W − X + XE₀)) + 1] × [C (X − XP₀)² + 1]" << std::endl;
    
    //print_state(BUSYFIT_FREE_PARAM, iter, s);
    
    // Solve the chi^2 minimisation problem
    do
    {
        iter++;
        status = gsl_multifit_fdfsolver_iterate(s);
        
        //std::cout << "Status: " << gsl_strerror(status) << std::endl;
        
        //print_state(BUSYFIT_FREE_PARAM, iter, s);
        
        if(status) break;
        
        status = gsl_multifit_test_delta(s->dx, s->x, 1.0e-4, 1.0e-4);
    }
    while(status == GSL_CONTINUE and iter < iterMax);
    
    gsl_multifit_covar(s->J, 0.0, covar);
    
    int nFreePar = 0;
    
    for(int i = 0; i < BUSYFIT_FREE_PARAM; i++)
    {
        if(spectrum.mask[i] == true) nFreePar++;
    }
    
    double chi = gsl_blas_dnrm2(s->f);
    double dof = spectrum.nChannels - nFreePar;
    double cc  = GSL_MAX_DBL(1.0, chi / sqrt(dof));
    
    // Extract results:
    chiSquare = chi * chi / dof;
    a         = gsl_vector_get(s->x, 0);
    b1        = gsl_vector_get(s->x, 1);
    b2        = gsl_vector_get(s->x, 2);
    c         = gsl_vector_get(s->x, 3);
    xe0       = gsl_vector_get(s->x, 4);
    xp0       = gsl_vector_get(s->x, 5);
    w         = gsl_vector_get(s->x, 6);
    err_a     = cc * sqrt(gsl_matrix_get(covar, 0, 0));
    err_b1    = cc * sqrt(gsl_matrix_get(covar, 1, 1));
    err_b2    = cc * sqrt(gsl_matrix_get(covar, 2, 2));
    err_c     = cc * sqrt(gsl_matrix_get(covar, 3, 3));
    err_xe0   = cc * sqrt(gsl_matrix_get(covar, 4, 4));
    err_xp0   = cc * sqrt(gsl_matrix_get(covar, 5, 5));
    err_w     = cc * sqrt(gsl_matrix_get(covar, 6, 6));
    
    std::cout << "\nStatus: " << gsl_strerror(status) << "\n" << std::endl;
    
    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    gsl_rng_free(r);
    
    return status;
}



// Definition of function to be fitted
// (This function is static!)

int BusyFit::expb_f(const gsl_vector *x, void *d, gsl_vector *f)
{
    // For B(x):
    double a   = gsl_vector_get(x, 0);
    double b1  = gsl_vector_get(x, 1);
    double b2  = gsl_vector_get(x, 2);
    double c   = gsl_vector_get(x, 3);
    double xe0 = gsl_vector_get(x, 4);
    double xp0 = gsl_vector_get(x, 5);
    double w   = gsl_vector_get(x, 6);
    
    struct data
    {
        size_t  nChannels;
        double *values;
        double *sigma;
        bool    mask[BUSYFIT_FREE_PARAM];
    };
    
    size_t  nChan = ((struct data *)d)->nChannels;
    double *y     = ((struct data *)d)->values;
    double *sigma = ((struct data *)d)->sigma;
    
    for(size_t i = 0; i < nChan; i++)
    {
        double ii = static_cast<double>(i);
        double Yi = (a / 4.0) * (erf(b1 * (w + ii - xe0)) + 1.0) * (erf(b2 * (w + xe0 - ii)) + 1.0) * (c * (ii - xp0) * (ii - xp0) + 1.0);
        
        gsl_vector_set(f, i, (Yi - y[i]) / sigma[i]);
    }
    
    return GSL_SUCCESS;
}

int BusyFit::expb_f2(const gsl_vector *x, void *d, gsl_vector *f)
{
    // For B2(x):
    double a   = gsl_vector_get(x, 0);
    double b1  = gsl_vector_get(x, 1);
    double b2  = gsl_vector_get(x, 2);
    double c   = gsl_vector_get(x, 3);
    double xe0 = gsl_vector_get(x, 4);
    double xp0 = gsl_vector_get(x, 5);
    double w   = gsl_vector_get(x, 6);
    
    struct data
    {
        size_t  nChannels;
        double *values;
        double *sigma;
        bool    mask[BUSYFIT_FREE_PARAM];
    };
    
    size_t  nChan = ((struct data *)d)->nChannels;
    double *y     = ((struct data *)d)->values;
    double *sigma = ((struct data *)d)->sigma;
    
    for(size_t i = 0; i < nChan; i++)
    {
        double ii = static_cast<double>(i);
        double Yi = (a / 4.0) * (erf(b1 * (w + ii - xe0)) + 1.0) * (erf(b2 * (w + xe0 - ii)) + 1.0) * (c * (ii - xp0) * (ii - xp0) * (ii - xp0) * (ii - xp0) + 1.0);
        
        gsl_vector_set(f, i, (Yi - y[i]) / sigma[i]);
    }
    
    return GSL_SUCCESS;
}



// Definition of Jacobian matrix of function to be fitted
// (This function is static!)

int BusyFit::expb_df(const gsl_vector *x, void *d, gsl_matrix *J)
{
    // For B(x):
    double a   = gsl_vector_get(x, 0);
    double b1  = gsl_vector_get(x, 1);
    double b2  = gsl_vector_get(x, 2);
    double c   = gsl_vector_get(x, 3);
    double xe0 = gsl_vector_get(x, 4);
    double xp0 = gsl_vector_get(x, 5);
    double w   = gsl_vector_get(x, 6);
    
    struct data
    {
        size_t  nChannels;
        double *values;
        double *sigma;
        bool    mask[BUSYFIT_FREE_PARAM];
    };
    
    size_t  nChan = ((struct data *)d)->nChannels;
    double *sigma = ((struct data *)d)->sigma;
    int     mask[BUSYFIT_FREE_PARAM];
    
    for(int i = 0; i < BUSYFIT_FREE_PARAM; i++)
    {
        mask[i] = ((struct data *)d)->mask[i];
    }
    
    for(size_t i = 0; i < nChan; i++)
    {
        // Jacobian matrix J(i,j) = dfi / dxj,
        // where fi = (Yi - yi)/sigma[i],
        //       Yi = value of function to be fitted
        // and the xj are the free parameters
        
        double ii = static_cast<double>(i);
        
        double erf1 = erf(b1 * (w + ii - xe0)) + 1.0;
        double erf2 = erf(b2 * (w + xe0 - ii)) + 1.0;
        double para = c * (ii - xp0) * (ii - xp0) + 1.0;
        double exp1 = exp(-(w + ii - xe0) * (w + ii - xe0) * b1 * b1);
        double exp2 = exp(-(w - ii + xe0) * (w - ii + xe0) * b2 * b2);
        
        gsl_matrix_set(J, i, 0, static_cast<double>(mask[0]) * (1.0 / 4.0) *       erf1 *  erf2 * para / sigma[i]); 
        gsl_matrix_set(J, i, 1, static_cast<double>(mask[1]) * (a / (2.0 * sqrt(M_PI))) *  erf2 * para * (w + ii - xe0) * exp1 / sigma[i]);
        gsl_matrix_set(J, i, 2, static_cast<double>(mask[2]) * (a / (2.0 * sqrt(M_PI))) *  erf1 * para * (w - ii + xe0) * exp2 / sigma[i]);
        gsl_matrix_set(J, i, 3, static_cast<double>(mask[3]) * (a / 4.0) *         erf1 *  erf2 * (xp0 - ii) * (xp0 - ii) / sigma[i]);
        gsl_matrix_set(J, i, 4, static_cast<double>(mask[4]) * (a / (2.0 * sqrt(M_PI))) * (erf1 * para * b2 * exp2 - erf2 * para * b1 * exp1) / sigma[i]);
        gsl_matrix_set(J, i, 5, static_cast<double>(mask[5]) * (a / 2.0) *         erf1 *  erf2 * (xp0 - ii) * c / sigma[i]);
        gsl_matrix_set(J, i, 6, static_cast<double>(mask[6]) * (a / (2.0 * sqrt(M_PI))) * (erf1 * para * b2 * exp2 + erf2 * para * b1 * exp1) / sigma[i]);
    }
    
    return GSL_SUCCESS;
}

int BusyFit::expb_df2(const gsl_vector *x, void *d, gsl_matrix *J)
{
    // For B2(x):
    double a   = gsl_vector_get(x, 0);
    double b1  = gsl_vector_get(x, 1);
    double b2  = gsl_vector_get(x, 2);
    double c   = gsl_vector_get(x, 3);
    double xe0 = gsl_vector_get(x, 4);
    double xp0 = gsl_vector_get(x, 5);
    double w   = gsl_vector_get(x, 6);
    
    struct data
    {
        size_t  nChannels;
        double *values;
        double *sigma;
        bool    mask[BUSYFIT_FREE_PARAM];
    };
    
    size_t  nChan = ((struct data *)d)->nChannels;
    double *sigma = ((struct data *)d)->sigma;
    int     mask[BUSYFIT_FREE_PARAM];
    
    for(int i = 0; i < BUSYFIT_FREE_PARAM; i++)
    {
        mask[i] = ((struct data *)d)->mask[i];
    }
    
    for(size_t i = 0; i < nChan; i++)
    {
        // Jacobian matrix J(i,j) = dfi / dxj,
        // where fi = (Yi - yi)/sigma[i],
        //       Yi = value of function to be fitted
        // and the xj are the free parameters
        
        double ii = static_cast<double>(i);
        
        double erf1 = erf(b1 * (w + ii - xe0)) + 1.0;
        double erf2 = erf(b2 * (w + xe0 - ii)) + 1.0;
        double para = c * (ii - xp0) * (ii - xp0) * (ii - xp0) * (ii - xp0) + 1.0;
        double exp1 = exp(-(w + ii - xe0) * (w + ii - xe0) * b1 * b1);
        double exp2 = exp(-(w - ii + xe0) * (w - ii + xe0) * b2 * b2);
        
        gsl_matrix_set(J, i, 0, static_cast<double>(mask[0]) * (1.0 / 4.0) *       erf1 *  erf2 * para / sigma[i]); 
        gsl_matrix_set(J, i, 1, static_cast<double>(mask[1]) * (a / (2.0 * sqrt(M_PI))) *  erf2 * para * (w + ii - xe0) * exp1 / sigma[i]);
        gsl_matrix_set(J, i, 2, static_cast<double>(mask[2]) * (a / (2.0 * sqrt(M_PI))) *  erf1 * para * (w - ii + xe0) * exp2 / sigma[i]);
        gsl_matrix_set(J, i, 3, static_cast<double>(mask[3]) * (a / 4.0) *         erf1 *  erf2 * (xp0 - ii) * (xp0 - ii) * (xp0 - ii) * (xp0 - ii) / sigma[i]);
        gsl_matrix_set(J, i, 4, static_cast<double>(mask[4]) * (a / (2.0 * sqrt(M_PI))) * (erf1 * para * b2 * exp2 - erf2 * para * b1 * exp1) / sigma[i]);
        gsl_matrix_set(J, i, 5, static_cast<double>(mask[5]) *  a        *         erf1 *  erf2 * (xp0 - ii) * (xp0 - ii) * (xp0 - ii) * c / sigma[i]);
        gsl_matrix_set(J, i, 6, static_cast<double>(mask[6]) * (a / (2.0 * sqrt(M_PI))) * (erf1 * para * b2 * exp2 + erf2 * para * b1 * exp1) / sigma[i]);
    }
    
    return GSL_SUCCESS;
}



// Function to call expb_f() and expb_df()
// (This function is static!)

int BusyFit::expb_fdf(const gsl_vector *x, void *d, gsl_vector *f, gsl_matrix *J)
{
    // For B(x):
    expb_f(x, d, f);
    expb_df(x, d, J);
    
    return GSL_SUCCESS;
}

int BusyFit::expb_fdf2(const gsl_vector *x, void *d, gsl_vector *f, gsl_matrix *J)
{
    // For B2(x):
    expb_f2(x, d, f);
    expb_df2(x, d, J);
    
    return GSL_SUCCESS;
}



// Function to print results of each iteration

void BusyFit::print_state(size_t p, size_t iter, gsl_multifit_fdfsolver *s)
{
    std::cout << "It " << iter << ":\t" << std::setprecision(4);
    
    for(size_t i = 0; i < p; i++)
    {
        std::cout << gsl_vector_get(s->x, i) << " ";
    }
    
    std::cout << "|F(x)| = " << gsl_blas_dnrm2(s->f) << std::endl;
    
    return;
}



// Busy Function, B(x), and derivative, dB(x)/dx:

double BusyFit::B(double x, int order)
{
    if(order == 4)
    {
        // Version with 4th-order polynomial trough:
        return (a / 4.0) * (erf(b1 * (w + x - xe0)) + 1.0) * (erf(b2 * (w + xe0 - x)) + 1.0) * (c * (x - xp0) * (x - xp0) * (x - xp0) * (x - xp0) + 1.0);
    }
    
    // Version with parabolic trough:
    return (a / 4.0) * (erf(b1 * (w + x - xe0)) + 1.0) * (erf(b2 * (w + xe0 - x)) + 1.0) * (c * (x - xp0) * (x - xp0) + 1.0);
}

/*double BusyFit::dB(double x)
{
    return (-a / 2.0) * (erf((w + x - xe0) * b1) + 1.0) * (erf((w - x + xe0) * b2) + 1.0) * (xp0 - x) * c - (a / 2.0) * (erf((w + x - xe0) * b1) + 1.0) * ((xp0 - x) * (xp0 - x) * c + 1.0) * b2 * exp(-(w - x + xe0) * (w - x + xe0) * b2 * b2) / sqrt(MATH_CONST_PI) + (a / 2.0) * (erf((w - x + xe0) * b2) + 1.0) * ((xp0 - x) * (xp0 - x) * c + 1.0) * b1 * exp(-(w + x - xe0) * (w + x - xe0) * b1 * b1) / sqrt(MATH_CONST_PI);
}*/

