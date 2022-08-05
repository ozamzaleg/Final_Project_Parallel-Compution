build:
	mpicc -fopenmp main.c functions.c  -o final_project -lm

clean:
	rm -f *.o ./final_project

run:
	mpiexec -np 2 ./final_project > output.txt

runOn2:
	mpiexec -np 2 -hostfile myHost.txt ./final_project > output.txt