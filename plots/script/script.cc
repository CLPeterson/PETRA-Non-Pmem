//petra/compile: make CPPFLAGS="-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LR_PTM"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#define TRANS_SIZE 4

FILE *pFile;
FILE *petraFile;
FILE *romulusFile;
FILE *onefileFile;

int main(int argc,char* argv[]) 
{ 
	//system("[ -f ../../compile/output.txt ] && rm ../../compile/output.txt");
	//system("[ -f output.txt ] && rm output.txt");
	system("[ -f outputPetra.txt ] && rm outputPetra.txt");
	system("[ -f outputRomulus.txt ] && rm outputRomulus.txt");
	system("[ -f outputOnefile.txt ] && rm outputOnefile.txt");

	std::string command;
	std::string command_suffix;

	std::string executablePetra("../../compile/src/./trans");
	std::string executableRomulus("../../compile/src/./transRomulus");
	std::string executableOnefile("../../compile/src/./transOneFile");
	std::string test_size;
	std::string key_range;
	std::string data_structure;
	std::string num_threads;   

	test_size.append(" 10000");
	key_range.append(" 10000");

	for(unsigned int i = 0; i < 4; i++)
	//for(unsigned int i = 0; i < 1; i++)
	{
		data_structure.clear();
		if(i == 0)
		{
			data_structure.append(" 0 ");
		} else if (i == 1)
		{
			data_structure.append(" 1 ");
		} else if (i == 2)
		{
			data_structure.append(" 2 ");
		} else if (i == 3)
		{
			data_structure.append(" 3 ");
		} 

		//for(unsigned int j = 0; j < 4; j++)
		for(unsigned int j = 0; j < 6; j++)
		{
			num_threads.clear();
			if(j == 0)
			{
				num_threads.append("1");
			} else if (j == 1)
			{
				num_threads.append("2");
			} else if (j == 2)
			{
				num_threads.append("4");
			} else if (j == 3)
			{
				num_threads.append("8");
			} else if (j == 4)
			{
				num_threads.append("16");
			} else if (j == 5)
			{
				num_threads.append("48");
			}
			
			command_suffix.clear();
			command_suffix.append(data_structure);
			command_suffix.append(num_threads);
			command_suffix.append(test_size);
			char trans_size[4];
			sprintf(trans_size, " %d", TRANS_SIZE);
			command_suffix.append(trans_size);
			command_suffix.append(key_range);
			command_suffix.append(" 33 33");

			for(unsigned int k = 0; k < 3; k++)
			{
				if(k == 0) {
					petraFile = fopen("outputPetra.txt", "a");
					if (petraFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					fprintf(petraFile, "%s ", num_threads.c_str());
					fclose(petraFile);


					command.clear();
					command.append(executablePetra);
					command.append(command_suffix);

					system(command.c_str());
					printf("%s\n", command.c_str());

				    pFile = fopen ("output.txt","r");
					if (pFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					double elapsedTime;
					fscanf (pFile, "%lf", &elapsedTime);
					unsigned int committed;
					fscanf (pFile, "%u", &committed);
					
					petraFile = fopen("outputPetra.txt", "a");
					if (petraFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					//fprintf(petraFile, " %.15lf\n", elapsedTime);
					fprintf(petraFile, " %.2lf\n", committed*TRANS_SIZE/elapsedTime);
					fclose(petraFile);

				} else if (k == 1) {
					romulusFile = fopen("outputRomulus.txt", "a");
					if (romulusFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					fprintf(romulusFile, "%s ", num_threads.c_str());
					fclose(romulusFile);

					command.clear();
					command.append(executableRomulus);
					command.append(command_suffix);

					system(command.c_str());
					printf("%s\n", command.c_str());

				    pFile = fopen ("output.txt","r");
					if (pFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					double elapsedTime;
					fscanf (pFile, "%lf", &elapsedTime);
					unsigned int committed;
					fscanf (pFile, "%u", &committed);
					
					romulusFile = fopen("outputRomulus.txt", "a");
					if (romulusFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					fprintf(romulusFile, " %.2lf\n", committed*TRANS_SIZE/elapsedTime);
					fclose(romulusFile);

				} else if (k == 2) {
					onefileFile = fopen("outputOnefile.txt", "a");
					if (onefileFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					fprintf(onefileFile, "%s ", num_threads.c_str());
					fclose(onefileFile);


					command.clear();
					command.append(executableOnefile);
					command.append(command_suffix);

					system(command.c_str());
					printf("%s\n", command.c_str());

				    pFile = fopen ("output.txt","r");
					if (pFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					double elapsedTime;
					fscanf (pFile, "%lf", &elapsedTime);
					unsigned int committed;
					fscanf (pFile, "%u", &committed);
					
					onefileFile = fopen("outputOnefile.txt", "a");
					if (onefileFile == NULL) {
						perror("fopen()");
						return EXIT_FAILURE;
					}
					fprintf(onefileFile, " %.2lf\n", committed*TRANS_SIZE/elapsedTime);
					fclose(onefileFile);
				}
				
			}
			
		}

		//system("sudo chmod 777 output.txt");
		petraFile = fopen("outputPetra.txt", "a");
		if (petraFile == NULL) {
			perror("fopen()");
			return EXIT_FAILURE;
		}
		fprintf(petraFile, "\n\n");
		fclose(petraFile);

		romulusFile = fopen("outputRomulus.txt", "a");
		if (romulusFile == NULL) {
			perror("fopen()");
			return EXIT_FAILURE;
		}
		fprintf(romulusFile, "\n\n");
		fclose(romulusFile);

		onefileFile = fopen("outputOnefile.txt", "a");
		if (onefileFile == NULL) {
			perror("fopen()");
			return EXIT_FAILURE;
		}
		fprintf(onefileFile, "\n\n");
		fclose(onefileFile);

	}

	//system("gnuplot -persist dynamic.p");

	//system("cp output.txt outputPetra.txt");

	return 0; 
}

