#include "coremap.hh"
#include "threads/system.hh"

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

int Coremap::Find(int virtualPage, OpenFile *currentSwapFile, TranslationEntry *currentPageTable){
   int ppn = physicals->Find();
   if(ppn < 0) { 
      // no hay espacio en memoria
      ppn = order->Pop();
      CoremapEntry entry = GetEntry(ppn);
      DEBUG('v', "En la PPN: %d estaba la VPN: %d del proceso %s.- Ahora se guardó la VPN: %d del %s\n",
               ppn, entry.vpn, entry.thread->GetName(), virtualPage, currentThread->GetName());
      // ASSERT((entries[ppn].thread)->space != nullptr);
      // hay que swappear
      saveInSwap(ppn, currentSwapFile, currentPageTable );
      // currentThread->space->loadFromSwap(ppn, virtualPage);
   } else {
      DEBUG('v', "No hizo falta SWAP - Thread: %s, PPN: %d, VPN: %d\n",
         currentThread->GetName(), ppn, virtualPage);
   }
   // habia lugar, o se hizo lugar con swap
   entries[ppn].thread = currentThread;
   entries[ppn].vpn = virtualPage;
   order->Append(ppn);
   return ppn;
}

void Coremap::saveInSwap(int ppn, OpenFile *currentSwapFile, TranslationEntry *currentPageTable){
   CoremapEntry entry = GetEntry(ppn);
   unsigned position = entry.vpn * PAGE_SIZE;
   if (currentThread == entry.thread){
      currentSwapFile->WriteAt(&machine->GetMMU()->mainMemory[ppn], PAGE_SIZE, position);
      currentPageTable[entry.vpn].valid = false;
   } else {
      entry.thread->space->getSwapFile()->WriteAt(&machine->GetMMU()->mainMemory[ppn], PAGE_SIZE, position);
      entry.thread->space->getPageTable()[entry.vpn].valid = false;
   }
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