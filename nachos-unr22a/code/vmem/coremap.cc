#include "coremap.hh"
#include <limits.h>
#include "threads/system.hh"

#ifdef USE_SWAP
Coremap::Coremap(unsigned pages){
   numPages = pages;
   physicals = new Bitmap(pages);
   order = new List<unsigned>();
   entries = new Entry[pages];
}

Coremap::~Coremap() {
   delete physicals;
   delete [] entries;
   for (int i=0; i<numPages; i++){
      // borrar entries
   } 

}

unsigned Coremap::Find(unsigned virtualPage){
   int page = physicals->Find();
   if(page < 0) { // no hay espacio en physicals
      page = order->Pop();
      DEBUG('v', "El proceso es %s\n",entries[page].process->GetName());
      DEBUG('v', "La ppn %d y la vpn %d\n", page, entries[page].virtualPage);
      ASSERT(entries[page].process->space != nullptr);
      entries[page].process->space->SwapPage(entries[page].virtualPage);
   }
   entries[page].process = currentThread;
   entries[page].virtualPage = virtualPage;
   order->Append(page);
   return page;
}

void Coremap::Clear(unsigned physicalPage){
   if(!physicals->Test(physicalPage)){
      return;
   }
   order->Remove(physicalPage);
   physicals->Clear(physicalPage);
}

void Coremap::Get(unsigned physicalPage){
   if(!physicals->Test(physicalPage)){
      return;
   }
   #ifdef COREMAP_LRU
   order->Remove(physicalPage);
   order->Append(physicalPage);
   #endif

   return;
}

#endif