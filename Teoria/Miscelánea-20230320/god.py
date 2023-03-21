#!/usr/bin/python -tt

#matmult, dim = 50,  memory_size = 8 -> 133.972 swaps (optimal)
#Number of memory accesses: 29.389.051

#Optimal Hard Hit ratio: 99,54414 %
#LRU Soft/Hard Hit ratio: 99,51971 %


#sort, size = 1024, memory_size = 8 -> 15.599 swaps (optimal)
#Number of memory accesses: 104.465.383

#Optimal Hard Hit ratio: 99,98506 %
#LRU Hard Hit ratio: 99,98401 %
import sys

def main():

    if len(sys.argv) < 3: 
        print "Uso: " , sys.argv[0], "Nombre-de-archivo Numero-de-paginas"
        return 0

    access_file = sys.argv[1]
    memory_size = int(sys.argv[2])
    if memory_size < 2: 
        print "Numero de paginas debe ser mayor a 1."
        return 0

    try:
        content = open(access_file,'r')
    except IOError:
        print 'Error abriendo el archivo'
        return

    # print [int(s[0:-1]) for s in content]
    accesses = [int(s[0:-1]) for s in content]
    content.close()

    print 'Lectura de archivo "', access_file, '" finalizada: ' , len(accesses), ' accesos.'
    print 'Numero de paginas fisicas: ', memory_size
    pages = 0
    for a in accesses:
        if a > pages:pages = a
    pages+= 1

    swaps = 0
    memory = []
    for i in range(0,len(accesses)):
        a = accesses[i]

        # print memory
        if i % 10000 == 0: print "Accesos analizados:" + str(i)
        if memory.count(a) > 0:
            continue

        if len(memory) < memory_size:
            memory.append(a)
            continue

        swap_val = last_used(accesses,i,memory)
        for i in range(0,memory_size):
            if memory[i] == swap_val:swap_frame = i
        memory[swap_frame] = a
        swaps += 1

    print 'Cantidad de accesos a memoria:', len(accesses)
    print "Swaps " + str(swaps)
    print "Hit Ratio : ", 100.0 - 100*(float(swaps)/float(len(accesses)))

def last_used(l,start,mem):
    remaining = mem[:]
    for i in range(start,len(l)):
        a = l[i]
        if remaining.count(a) > 0:
            remaining.remove(a)
            if len(remaining) == 1:
                return remaining[0]

    return remaining[0]
        

if __name__ == '__main__': main()
