//
//  mean_variance.h
//  cpputils
//
//  Created by Javier Valcarce on 29/03/14.
//  Copyright (c) 2014 Javier Valcarce. All rights reserved.
//

#ifndef __cpputils__mean_variance__
#define __cpputils__mean_variance__




class MeanVariance
{
      
private:
      
      double m_min;
      double m_max;
      double m_acc;  // accumulates values xi
      double m_acc2; // accumulates squared values xi^2
      double m_last;
      
      
      int m_count;
      
      
public:
      
      MeanVariance();
      
      /**
       *    Reset all accumulators and the object's state
       */
      void Reset();
      
      /**
       * Feed estimator with one sample
       */
      void Compute(double sample);
      
      /**
       * Feed estimator with an array of samples
       */
      void Compute(double* sample_vector, int len);
      
      /**
       * Minimun value
       */
      double Min();
      
      /**
       * Maximum value
       */
      double Max();
      
      /**
       * Last value feeded
       */
      double Last();
      
      /**
       * Current mean estimation
       */
      double Mean();
      
      /**
       * Current variance estimation
       */
      double Variance();
  
      /**
       * Number of samples processed
       */
      int Count();
      
      
};


#endif /* defined(__cpputils__mean_variance__) */
 
