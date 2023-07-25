#ifndef NACHOS_THREADS_COREMAP__HH
#define NACHOS_THREADS_COREMAP__HH

#include "lib/bitmap.hh"
#include "threads/thread.hh"
#include "lib/list.hh"
#ifdef SWAP

class CoremapEntry {
   public:
      // nos falta un constructor y un destructor
      Thread *thread;
      unsigned vpn;
};


class Coremap {
   public:
      Coremap(unsigned numPages);

      ~Coremap();
      
      CoremapEntry Find(unsigned virtualPage);
      
      void Clear(unsigned virtualPage);
      
      void Get(unsigned physicalPage);
   
   private:
      Bitmap *physicals;      // bitmap de páginas físicas
      CoremapEntry *entries;  // par <thread,vpn> al que pertenece cada ppn
      List<unsigned> *order;  // orden en el que vamos a ir liberando las páginas
      unsigned numPages;      // cantidad de páginas físicas (NUM_PAGES)
};


#endif
#endif