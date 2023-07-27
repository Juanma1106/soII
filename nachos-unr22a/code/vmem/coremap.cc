#include "coremap.hh"

#ifdef SWAP
Coremap::Coremap(unsigned pages){
   numPages = pages;
   physicals = new Bitmap(pages);
   order = new List<unsigned>();
   entries = new CoremapEntry[pages];
}

Coremap::~Coremap() {
   delete physicals;
   delete [] entries;
   for (unsigned int i=0; i<numPages; i++){
      // borrar entries
   } 

}

unsigned Coremap::Find(unsigned virtualPage, Thread *currentThread){
   unsigned int ppn = physicals->Find();
   if(ppn < 0) { 
      // no hay espacio en memoria
      ppn = order->Pop();
      DEBUG('v', "El proceso es %s\n",entries[ppn].thread->GetName());
      DEBUG('v', "La ppn %d y la vpn %d\n", ppn, entries[ppn].vpn);
      ASSERT(entries[ppn].thread->space != nullptr);
      // hay que swappear
      entries[ppn].thread->space->saveInSwap(ppn);
   }
   // habia lugar, o se hizo lugar con swap
   entries[ppn].thread = currentThread;
   entries[ppn].vpn = virtualPage;
   order->Append(ppn);
   return ppn;
}

void Coremap::Clear(unsigned ppn){
   if(!physicals->Test(ppn)){
      return;
   }
   // si estaba ocupada, se libera
   order->Remove(ppn);
   physicals->Clear(ppn);
}

CoremapEntry Coremap::GetEntry(unsigned ppn){
   ASSERT(ppn<numPages);
   return entries[ppn];
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