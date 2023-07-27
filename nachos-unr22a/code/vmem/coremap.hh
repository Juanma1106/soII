#ifndef NACHOS_VMEM_COREMAP__HH
#define NACHOS_VMEM_COREMAP__HH

#include "lib/bitmap.hh"
#include "lib/list.hh"
#include "threads/thread.hh"
#include "userprog/address_space.hh"


#ifdef SWAP
class CoremapEntry {
   public:
      // nos falta un constructor y un destructor ?
      Thread *thread;
      unsigned vpn;
};


class Coremap {
   public:
      Coremap(unsigned numPages);

      ~Coremap();
      
      unsigned Find(unsigned virtualPage, Thread *currentThread);
      
      void Clear(unsigned virtualPage);
      
      void Get(unsigned physicalPage);
   
      CoremapEntry GetEntry(unsigned ppn);

   private:
      Bitmap *physicals;      // bitmap de páginas físicas
      CoremapEntry *entries;  // par <thread,vpn> al que pertenece cada ppn
      List<unsigned> *order;  // orden en el que vamos a ir liberando las páginas
      unsigned numPages;      // cantidad de páginas físicas (NUM_PHYS_PAGES)
};


#endif
#endif