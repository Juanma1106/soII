#include "coremap.hh"
#include <limits.h>
#include "threads/system.hh"

#ifdef USE_SWAP
Coremap::Coremap(unsigned pages){
   numPages = pages;
   physicals = new Bitmap(pages);
   order = new List<unsigned>();
   entries = new CoremapEntry[pages];
}

Coremap::~Coremap() {
   delete physicals;
   delete [] entries;
   for (int i=0; i<numPages; i++){
      // borrar entries
   } 

}

CoremapEntry Coremap::Find(unsigned virtualPage){
   int ppn = physicals->Find();
   if(ppn < 0) { 
      // no hay espacio en memoria
      ppn = order->Pop();
      DEBUG('v', "El proceso es %s\n",entries[ppn].process->GetName());
      DEBUG('v', "La ppn %d y la vpn %d\n", ppn, entries[ppn].vpn);
      ASSERT(entries[ppn].process->space != nullptr);
      // hay que swappear
      entries[ppn].process->space->saveInSwap(ppn);
   }
   // habia lugar, o se hizo lugar con swap
   entries[ppn].process = currentThread;
   entries[ppn].vpn = virtualPage;
   order->Append(ppn);
   return entries[ppn];
}

void Coremap::Clear(unsigned ppn){
   if(!physicals->Test(ppn)){
      return;
   }
   // si estaba ocupada, se libera
   order->Remove(ppn);
   physicals->Clear(ppn);
}

void Coremap::Get(unsigned ppn){
   if(!physicals->Test(ppn)){
      return;
   }
   // #ifdef COREMAP_LRU
   // order->Remove(ppn);
   // order->Append(ppn);
   // #endif

   return;
}

#endif