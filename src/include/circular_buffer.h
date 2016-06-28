// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_CIRCULAR_BUFFER_RX_
#define TELETRAFFIC_CIRCULAR_BUFFER_RX_

//
// circular_buffer.h - Simple circular buffer with [] operator, not synchronized
//
// Javier Valcarce García
//
// Since this is a template class the implementation goes in the header file
// there is no cpp file
#include <stdexcept>
#include <cstdio>
#include <cassert>


namespace teletraffic {

template <class T> 
class CircularBuffer
{        
      T*              m_buffer;
      //int             m_first;
      int             m_next;
      int             m_count;
      int             m_capacity;

public:
      
      CircularBuffer(int capacity); 

      //CircularBuffer(const CircularBuffer& a); // copy constructor ¡Hay que definirlo para un T concreto y no se hace
      //aquí sino el el .cpp de la aplicación! Eso es porque esto es una plantilla y no una clase concreta normal.

      ~CircularBuffer(); 
      
      /** Añade un elemento a la cola */
      int  Push(const T& val);  

      /** Vacia el búfer */
      void Clear();

      /** Número de elementos actualmente en cola */
      int  Size();

      /** Capacidad de la cola (número máximo de elementos que puede albergar) */
      int  Capacity();
      
      /** operador [], [0] es el último elemento introducido, [1] es el penúltimo, [2] es el antepenúltimo, etc */
      T& operator[](int idx);
      //const value_type& operator[](index_type idx) const;

      /** 
       * Lo mismo que [] pero con comprobación de rango, si el índice está fuera de rango genera la excepción
       * std::out_of_range
       */
      T& At(int idx);
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> CircularBuffer<T>::CircularBuffer(int capacity) {
      assert(capacity > 0);
      m_capacity = capacity;
      m_next = 0;
      m_count = 0;
      m_buffer = new T[capacity];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> CircularBuffer<T>::~CircularBuffer() {
      delete[] m_buffer; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> void CircularBuffer<T>::Clear() {
      m_next = 0;
      m_count = 0;
}              

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Size() {
      return m_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Capacity() {
      return m_capacity;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int CircularBuffer<T>::Push(const T& val) {
      m_buffer[m_next] = val; // copia del valor, no de la referencia
      m_next++;

      if (m_next == m_capacity) {
            m_next = 0;
      }

      if (m_count < m_capacity) {
            m_count++;
      } 

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T& CircularBuffer<T>::operator[](int idx) {
      assert(idx < m_count);

      int r;
      int i;

      r = m_next - idx - 1;
      i = r < 0 ? r + m_capacity : r;

      return m_buffer[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T& CircularBuffer<T>::At(int idx) {
      if (idx < m_count) {
            throw std::out_of_range("CircularBuffer<T>::At out of range");
      }

      int r;
      int i;

      r = m_next - idx - 1;
      i = r < 0 ? r + m_capacity : r;

      return m_buffer[i];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



}  // namespace teletraffic


#endif  // TELETRAFFIC_CIRCULAR_BUFFER_RX_
