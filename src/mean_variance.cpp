// Hi Emacs, this is -*- mode: c++; c-basic-offset: 6; c-file-style: "linux"; -*-
#include "mean_variance.h"
#include <limits>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeanVariance::MeanVariance() {
      Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MeanVariance::Reset() {
      m_min = std::numeric_limits<double>::max();
      m_max = std::numeric_limits<double>::min();
      m_acc = 0.0;
      m_acc2 = 0.0;
      m_last = 0.0;
      m_count = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MeanVariance::Compute(double sample) {
      Compute(&sample, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MeanVariance::Compute(double* sample_vector, int len) {
      int i;
      
      for (i = 0; i < len; i++) {
            m_acc  += sample_vector[i];
            m_acc2 += sample_vector[i] * sample_vector[i];
            
            if (sample_vector[i] < m_min) {
                  m_min = sample_vector[i];
            }
            
            if (sample_vector[i] > m_max) {
                  m_max = sample_vector[i];
            }
      }
      
      m_count += len;
      m_last = sample_vector[len - 1];
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MeanVariance::Min() {
      if (m_count == 0) {
            return std::numeric_limits<double>::quiet_NaN();
      }

      return m_min;      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MeanVariance::Max() {
      if (m_count == 0) {
            return std::numeric_limits<double>::quiet_NaN();
      }
      return m_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MeanVariance::Last() {
      if (m_count == 0) {
            return std::numeric_limits<double>::quiet_NaN();
      }
      
      return m_last;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MeanVariance::Mean() {
      if (m_count == 0) {
            return std::numeric_limits<double>::quiet_NaN();
      }
      return m_acc / m_count;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MeanVariance::Variance() {
      double mu;
       
      // unbiased estimator of the variance (N-1)
      if (m_count == 0 || m_count == 1) {
            return std::numeric_limits<double>::quiet_NaN();
      }
      
      mu = m_acc / m_count;
      return (m_acc2 + m_count * mu * mu - 2* mu * m_acc) / (m_count - 1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MeanVariance::Count() {
      return m_count;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
