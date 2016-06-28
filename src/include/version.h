// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_VERSION_H_
#define TELETRAFFIC_VERSION_H_

#include <string>

/**
 * Main namespace, contains all types of the library 
 */
namespace teletraffic {

      /**
       * Returns the version's major number, i.e, if the version is "0.1.2" returns 0
       * This is the internal version number the library was compiled with
       * @return Version's major number
       */
      int VersionMajor();

      /**
       * Returns the version's minor number, i.e, if the version is "0.1.2" returns 1
       * This is the internal version number the library was compiled with
       * @return Version's minor number
       */
      int VersionMinor();

      /**
       * Returns the version's micro number, i.e, if the version is "0.1.2" returns 2
       * This is the internal version number the library was compiled with
       * @return Version's micro number
       */
      int VersionMicro();

      /**
       * Version string.
       */
      const char* VersionString();
      
      /**
       * Checks that the specified minimum required version is older than (and hence compatible) 
       * the version of the library.
       *
       * @param major Minimum required major number
       * @param minor Minimum required minor number
       * @param micro Minimum required micro number
       * @return true if the library is compatible with the version specified, false in other case.
       */
      bool VersionCheck(int major, int minor, int micro);

      /**
       * Fecha de la compilación de esta biblioteca.
       */
      const char* BuildDate();

      /**
       * Fecha en la que se hiz la etiqueta en SVN.
       */
      const char* PublicationDate();   
}

#endif // TELETRAFFIC_VERSION_H_
