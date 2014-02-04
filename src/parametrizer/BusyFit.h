/// ____________________________________________________________________ ///
///                                                                      ///
/// busyfit (BusyFit.h) - Busy Function fitting programme                ///
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

#ifndef BUSYFIT_H
#define BUSYFIT_H

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

#define BUSYFIT_FREE_PARAM 7

class BusyFit
{
public:
    BusyFit();
    
    int     setup(size_t n, double *newData, double *newSigma, int newOrder = 2, bool noPlot = false, bool relax = false);
    int     setFreeParameters(bool newMask[BUSYFIT_FREE_PARAM]);
    int     fit();
    int     fit(double new_a, double new_b1, double new_b2, double new_c, double new_xe0, double new_xp0, double new_w);
    int     getResult(double *parValues, double *parUncert, double &chi2);
    int     getParameters(double &posX, double &w50, double &w20, double &Fpeak, double &Fint);
    
private:
    struct data
    {
        size_t  nChannels;
        double *values;
        double *sigma;
        bool    mask[BUSYFIT_FREE_PARAM];
    } spectrum;
    
    double  init_a;
    double  init_b1, init_b2;
    double  init_c;
    double  init_xe0, init_xp0;
    double  init_w;
    
    double  a;
    double  b1, b2;
    double  c;
    double  xe0, xp0;
    double  w;
    
    double  err_a;
    double  err_b1, err_b2;
    double  err_c;
    double  err_xe0, err_xp0;
    double  err_w;
    
    double  chiSquare;
    int     order;
    
    bool    flagNoPlot;
    bool    flagRelax;
    
    int     fitWithEstimates();
    int     LMSolver();
    void    print_state(size_t p, size_t iter, gsl_multifit_fdfsolver *s);
    int     printResult();
    int     plotResult();
    double  B(double x, int order = 2);
    double  B2(double x);
    //double  dB(double x);
    
    // The following functions have to be static so they can be pointed to:
    static int expb_f   (const gsl_vector *x, void *d, gsl_vector *f);
    static int expb_f2  (const gsl_vector *x, void *d, gsl_vector *f);
    static int expb_df  (const gsl_vector *x, void *d, gsl_matrix *J);
    static int expb_df2 (const gsl_vector *x, void *d, gsl_matrix *J);
    static int expb_fdf (const gsl_vector *x, void *d, gsl_vector *f, gsl_matrix *J);
    static int expb_fdf2(const gsl_vector *x, void *d, gsl_vector *f, gsl_matrix *J);
};

#endif
