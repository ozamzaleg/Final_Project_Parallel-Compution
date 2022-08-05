Name: Oz Amzaleg , ID: 313554958

To run this program on terminal:

1. make - mpicc -fopenmp main.c functions.c -o final_project -lm

2. make run - mpiexec -np 2 ./final_project > output.txt

3. make runOn2 - mpiexec -np 2 -hostfile myHost.txt ./final_project > output.txt

Output:

Picture 0: found Object 1 in Position(100,200)

Picture 1: No Objects were found

Picture 2: found Object 0 in Position(800,800)
