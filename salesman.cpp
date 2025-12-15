// example code to read in a data file of city lat,long coordinates

#include <cstdio>
#include <cstdlib>

// simple structure to store city coordinates
// could also use std::pair<double> 
// or define a class

typedef struct {
  double lon, lat;
} COORD;

// fill the array of city locations
int GetData(char* fname, COORD *cities){
  FILE* fp=fopen(fname,"r");
  const int bufsiz=1000;
  char line[bufsiz+1];
  int ncity=0;
  while(1){
    fgets(line,bufsiz,fp);
    if (line[0]=='#') continue;  // skip comments
    if (feof(fp)) break;
    // we only scan for two numbers at start of each line
    sscanf(line,"%lf %lf",&cities[ncity].lon,&cities[ncity].lat);    
    ncity++;
  }
  fclose(fp);
  return ncity;
}

int main(int argc, char *argv[]){
  const int NMAX=2500;
  COORD cities[NMAX];

  if (argc<2){
    printf("Please provide a data file path as argument\n");
    return 1;
  }

  int ncity=GetData(argv[1],cities);
  printf("Read %d cities from data file\n",ncity);
  printf("Longitude  Latitude\n");
  for (int i=0; i<ncity; i++)
    printf("%lf %lf\n",	cities[i].lon,cities[i].lat); 
  return 0;
}

