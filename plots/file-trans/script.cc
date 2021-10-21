//petra/compile: make CPPFLAGS="-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LR_PTM"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

FILE *petraFile;
FILE *romulusFile;
FILE *onefileFile;

int main(int argc,char* argv[]) 
{ 
	while(true)
	{

		system("rsync -avh -e 'ssh -i ~/.ssh/id_rsa' christina@10.173.215.69:/home/christina/Documents/PETRA-Non-Pmem/plots/script/outputPetra.txt /home/christina/Documents/petra-non-pmem/plots/script");

		system("rsync -avh -e 'ssh -i ~/.ssh/id_rsa' christina@10.173.215.69:/home/christina/Documents/PETRA-Non-Pmem/plots/script/outputRomulus.txt /home/christina/Documents/petra-non-pmem/plots/script");

		system("rsync -avh -e 'ssh -i ~/.ssh/id_rsa' christina@10.173.215.69:/home/christina/Documents/PETRA-Non-Pmem/plots/script/outputOnefile.txt /home/christina/Documents/petra-non-pmem/plots/script");

		sleep(3);
	}

	return 0; 
}

