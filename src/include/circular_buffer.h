//
// circular_buffer.h - Simple circular buffer with [] operator, not synchronized
//
// Javier Valcarce Garc�a
//

// Since this is a template class the implementation goes in the header file
// there is no cpp file

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_
#include <stdexcept>

namespace teletraffic {

template <class T> 
class CircularBuffer
{        
      T*              m_buffer;
      int             m_last;
      int             m_count;
      int             m_capacity;

public:
      
      CircularBuffer(int capacity); 
      ~CircularBuffer(); 
      
      /** A�ade un elemento a la cola */
      int  Push(const T& val);  

      /** Vacia el b�fer */
      void Clear();

      /** N�mero de elementos actualmente en cola */
      int  Size();

      /** Capacidad de la cola (n�mero m�ximo de elementos que puede albergar) */
      int  Capacity();
      
      /** operador [], [0] es el �ltimo elemento introducido, [1] es el pen�ltimo, [2] es el antepen�ltimo, etc */
      T& operator[](int idx);
      //const value_type& operator[](index_type idx) const;

      /** 
       * Lo mismo que [] pero con comprobaci�n de rango, si el �ndice est� fuera de rango genera la excepci�n
       * std::out_of_range
       */
      T& At(int idx);
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T& CircularBuffer<T>::operator[](int idx) 
{
      int r = (m_last - idx);
      int i = r < 0 ? r + m_capacity : r;

      return m_buffer[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T& CircularBuffer<T>::At(int idx)
{
      int r = (m_last - idx);
      int i = r < 0 ? r + m_capacity : r;

      if (i < 0 || i >= m_capacity) 
      {
            // error, �ndica fuera de rango
            throw std::out_of_range("CircularBuffer<T>::At out of range");
      }

      return m_buffer[i];
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> CircularBuffer<T>::CircularBuffer(int capacity) 
{
      m_capacity = capacity;
      m_last = 0;
      m_count = 0;

      m_buffer = new T[capacity];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> CircularBuffer<T>::~CircularBuffer() 
{
      delete[] m_buffer; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Push(const T& val)
{
      m_buffer[m_last] = val; // copia del valor, no de la referencia
      m_last++;

      if (m_last >= m_capacity) 
      {
            m_last = 0;
      }

      if (m_count < m_capacity)
      {
            m_count++;
      }

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> void CircularBuffer<T>::Clear() 
{
      m_last = 0;
      m_count = 0;
}              

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Size() 
{
      return m_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Capacity() 
{
      return m_capacity;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif  // CIRCULAR_BUFFER_H_
