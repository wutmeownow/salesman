// example code to read in a data file of city lat,long coordinates

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <TRandom3.h>

using namespace std;

// constants for distance calculation
const double R = 6371.; // radius of earth


// simple structure to store city coordinates
// could also use std::pair<double> 
// or define a class

typedef struct {
  double lon, lat;
} COORD;

// calculate separation between two coordinates on globe
double GetDistance(const COORD i, const COORD j) {
  double dlat = i.lat - j.lat;
  double dlong = i.lon - j.lon;
  double a = pow(sin(dlat/2),2) + cos(i.lat)*cos(j.lat)*pow(sin(dlong/2),2);
  double c = 2*atan2(sqrt(a),sqrt(1-a));
  
  return R*c;
}

// get total distance of path around globe
double GetTotalDistance(int ncities, const COORD *cities) {
  double dist = 0.;
  for (int i=0;i<ncities;i++) {
    int j = i-1; // index for city before this one
    if (j<0) {j=ncities-1;} // starting city, city before is the last one
    dist += GetDistance(cities[i], cities[j]);
  }
  return dist;
}

void UpdatePath(int ncities, COORD *cities) {
  TRandom3 *r = new TRandom3(0);
  int iRand = 0; // starting index of slice
  int lRand = ncities-1; // slice length
  // swapping the entire range changes nothing, iterate until we get something smaller
  while (iRand == 0 && lRand==ncities-1) {
    iRand = r->Integer(ncities-1); // generate random starting index that CANNOT be final index
    lRand = r->Integer(ncities-iRand-1) + 1; // generate random slice from random index to end: (1, N-iRand-1) inclusive
  }
  cout<< "Slice to reverse: " << iRand << " to "<< iRand + lRand <<endl;

  // difference in path length will come from change in slice end pairs
  int jRand = iRand + lRand; // final index of range to reverse

  int ioldpartner = iRand-1; // index of partner city for slice start
  if (ioldpartner<0) {ioldpartner=ncities-1;} // less than one, partner is final city

  int joldpartner = jRand+1; // index of partner city for slice end
  if (joldpartner>ncities-1) {joldpartner=0;} // greater than ncities-1, partner is first city

  double dold = GetDistance(cities[iRand], cities[ioldpartner]) + GetDistance(cities[jRand], cities[joldpartner]);
  double dnew = GetDistance(cities[iRand], cities[joldpartner]) + GetDistance(cities[jRand], cities[ioldpartner]);
  double dL = dnew-dold; // change in path length for this random slice reversal
  
  printf("Change in trip distance: %.3f km\n",	dL);
}

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
    sscanf(line,"%lf %lf",& cities[ncity].lon,&cities[ncity].lat);    
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
  // printf("Longitude  Latitude\n");
  // for (int i=0; i<ncity; i++)
  //   printf("%lf %lf\n",	cities[i].lon,cities[i].lat); 

  double distance = GetTotalDistance(ncity,cities);
  printf("Total trip distance: %.2f km\n",	distance);
  for (int i=0; i<100;i++) {UpdatePath(ncity, cities);}
  return 0;
}

