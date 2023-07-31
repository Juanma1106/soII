#include "coremap.hh"

#ifdef SWAP
Coremap::Coremap(unsigned pages){
   numPages = pages;
   physicals = new Bitmap(pages);
   order = new List<int>();
   entries = new CoremapEntry[pages];
}

Coremap::~Coremap() {
   delete physicals;
   delete [] entries;
   for (unsigned int i=0; i<numPages; i++){
      // borrar entries
   } 

}

int Coremap::Find(int virtualPage,  Thread *currentThread, 
               OpenFile * currentSwapFile, TranslationEntry *pageTable){
   int ppn = physicals->Find();
   if(ppn < 0) { 
      // no hay espacio en memoria
      ppn = order->Pop();
      DEBUG('v', "El proceso es %s\n",entries[ppn].thread->GetName());
      DEBUG('v', "La ppn %d y la vpn %d\n", ppn, entries[ppn].vpn);
      CoremapEntry entry = GetEntry(ppn);
      OpenFile * swapFile;
      TranslationEntry *pgtable;
      if (currentThread == entry.thread){
         swapFile = currentSwapFile;
         pgtable = pageTable;
      } else {
         swapFile = entry.thread->space->getSwapFile();
         pgtable = entry.thread->space->getPageTable();
      }

      // ASSERT((entries[ppn].thread)->space != nullptr);
      // hay que swappear
      (entries[ppn].thread)->space->saveInSwap(ppn, swapFile, pgtable);
      currentThread->space->loadFromSwap(ppn, virtualPage);
   }
   // habia lugar, o se hizo lugar con swap
   entries[ppn].thread = currentThread;
   entries[ppn].vpn = virtualPage;
   order->Append(ppn);
   return ppn;
}

void Coremap::Clear(int ppn){
   if(!physicals->Test(ppn)){
      return;
   }
   // si estaba ocupada, se libera
   order->Remove(ppn);
   physicals->Clear(ppn);
}


void Coremap::Get(int ppn){
   if(!physicals->Test(ppn)){
      return;
   }
   // #ifdef COREMAP_LRU
   // order->Remove(ppn);
   // order->Append(ppn);
   // #endif

   return;
}

CoremapEntry Coremap::GetEntry(int ppn){
   ASSERT((unsigned) ppn<numPages);
   return entries[ppn];
}
#endif