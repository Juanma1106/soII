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
               OpenFile * currentSwapFile, TranslationEntry *pageTable,
               char * mainMemory ){
   int ppn = physicals->Find();
   if(ppn < 0) { 
      // no hay espacio en memoria
      ppn = order->Pop();
      CoremapEntry entry = GetEntry(ppn);
      DEBUG('v', "En la PPN: %d estaba la VPN: %d del proceso %s\n",
               ppn, entry.vpn, entry.thread->GetName());
      DEBUG('v', "Ahora se guardó la VPN: %d del %s\n",
               virtualPage, currentThread->GetName() );
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
      char from = mainMemory[ppn];
      saveInSwap(ppn, swapFile, pgtable, &from);
      // currentThread->space->loadFromSwap(ppn, virtualPage);
   } else {
      DEBUG('v', "No hizo falta SWAP - Thread: %s, PPN: %d, VPN: %d\n",
         currentThread->GetName(),
         ppn, virtualPage);
   }
   // habia lugar, o se hizo lugar con swap
   entries[ppn].thread = currentThread;
   entries[ppn].vpn = virtualPage;
   order->Append(ppn);
   return ppn;
}

void Coremap::saveInSwap(int ppn, OpenFile * swapFile, TranslationEntry *pgtable, char *from){
   CoremapEntry entry = GetEntry(ppn);
   unsigned position = entry.vpn * PAGE_SIZE;
   swapFile->WriteAt(from, PAGE_SIZE, position);
   pgtable[entry.vpn].valid = false;
   DEBUG('v', "SWAP - VPN: %d guardada en swap \n", entry.vpn);
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