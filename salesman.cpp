// example code to read in a data file of city lat,long coordinates

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <TGraph.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TStopwatch.h>
#include <unistd.h> // Required for getopt
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

double UpdatePath(int ncities, COORD *cities, TRandom3 *r, double T=-1.) {
  int iRand = 0; // starting index of slice
  int lRand = ncities-1; // slice length
  // swapping the entire range changes nothing, iterate until we get something smaller
  while (iRand == 0 && lRand==ncities-1) {
    iRand = r->Integer(ncities-1); // generate random starting index that CANNOT be final index
    lRand = r->Integer(ncities-iRand-1) + 1; // generate random slice from random index to end: (1, N-iRand-1) inclusive
  }
  // cout<< "Slice to reverse: " << iRand << " to "<< iRand + lRand <<endl;

  // difference in path length will come from change in slice end pairs
  int jRand = iRand + lRand; // final index of range to reverse

  int ioldpartner = iRand-1; // index of partner city for slice start
  if (ioldpartner<0) {ioldpartner=ncities-1;} // less than one, partner is final city

  int joldpartner = jRand+1; // index of partner city for slice end
  if (joldpartner>ncities-1) {joldpartner=0;} // greater than ncities-1, partner is first city

  double dold = GetDistance(cities[iRand], cities[ioldpartner]) + GetDistance(cities[jRand], cities[joldpartner]);
  double dnew = GetDistance(cities[iRand], cities[joldpartner]) + GetDistance(cities[jRand], cities[ioldpartner]);
  double dL = dnew-dold; // change in path length for this random slice reversal

  // carry out the change with std::reverse
  // Reverse indices iRand to jRand
  // Pointer arithmetic: arr + start, arr + end + 1
  if (T<0) {
    // hot, always take the change
    reverse(cities+iRand, cities+jRand+1);
    return dL;
  } 
  else {
    double p = r->Rndm();
    if (dL<0 || p < exp(-dL/T)) {
      reverse(cities+iRand, cities+jRand+1);
      return 1.; // successful change
    }
    return 0.; // no change
  }
  
  // printf("Change in trip distance: %.3f km\n",	dL);
  // return dL;
}


// fill the array of city locations
int GetData(string fname, COORD *cities){
  FILE* fp=fopen(fname.c_str(),"r");
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


void WriteData(const string& filename, int ncity, const COORD *cities, double d1, double d2) {
    // d1 & d2 are trip distances before and after annealing

    // Open the file stream for writing
    ofstream outFile(filename);

    // Check if file opened successfully
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    // Optional: Write a header line (useful for pandas/ROOT)
    outFile << "#longitude   latitude\n";

    // Loop through the data and write lines
    for (int i=0;i<ncity;i++) {
        outFile << cities[i].lon << "   " << cities[i].lat << "\n";
    }

    // add final line with total distance before and after for plotting later
    outFile << "D: " << d1 << "   " << d2 << "\n";

    outFile.close();
    std::cout << "Successfully wrote " << ncity << " points to " << filename << std::endl;
}


void PlotCoolingHistory(const std::vector<double> &v_temp, const std::vector<double> &v_dist, const string& filename) {
    // Create Canvas
    TCanvas *c2 = new TCanvas("c2", "Annealing History", 800, 600);
    
    // Set X-axis to Log Scale (Crucial for Annealing plots)
    c2->SetLogx(); 

    // Create Graph
    // &v_temp[0] gives the pointer to the underlying array of the vector
    TGraph *gr = new TGraph(v_temp.size(), &v_temp[0], &v_dist[0]);

    // Styling
    gr->SetTitle("Optimization History;Temperature (T);Total Distance [km]");
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(0.6);
    gr->SetMarkerColor(kBlue);
    gr->SetLineColor(kBlue);
    gr->SetLineWidth(2);

    // Draw with Lines (L) and Points (P) and Axis (A)
    // Note: Since T goes High -> Low, the graph draws Right -> Left. 
    // This is normal for physics annealing plots.
    gr->Draw("ALP");

    // Save File
    string imgFile = filename;
    size_t lastindex = imgFile.find_last_of("."); 
    string rawName = imgFile.substr(0, lastindex); 
    c2->SaveAs((rawName + "_history.pdf").c_str());

    delete gr;
    delete c2;
}


int main(int argc, char *argv[]){
  TStopwatch timer; // timer to track how long code takes
  timer.Start();
  const int NMAX=2500;
  COORD cities[NMAX];
  TRandom3 *r = new TRandom3(0); // rng

  // parameters
  string filename = ""; // Default value
  double alpha = 0.8; // next T parameter
  int N = 10; // multiplicative factor for number of trials
  double limit = 0.1; // limit factor for when to stop annealing
  int opt;

  // Loop through all arguments
  // "n:f:" means we expect flags -n and -f, and both require values (:)
  while ((opt = getopt(argc, argv, "f:n:a:l:")) != -1) {
      switch (opt) {
          case 'f':
            filename = optarg;
            break;
          case 'a':
            alpha = std::stof(optarg);
            break;
          case 'n':
            N = std::atoi(optarg);
            break;
          case 'l':
            limit = std::stof(optarg);
            break;
          default:
            std::cerr << "Usage: " << argv[0] << " -f <filename> -a <alpha> -n <N> -t <Tmin)" << std::endl;
            return 1;
      }
  }

  if (filename.empty()) {
      std::cerr << "Error: Filename (-f) is required!" << std::endl;
      return 1;
  }


  int ncity=GetData(filename,cities);
  printf("Read %d cities from data file\n",ncity);
  // printf("Longitude  Latitude\n");
  // for (int i=0; i<ncity; i++)
  //   printf("%lf %lf\n",	cities[i].lon,cities[i].lat); 

  double distance = GetTotalDistance(ncity,cities);
  printf("Total starting trip distance: %.2f km\n",	distance);

  // come up with a starting Tmax by randomly varying path and taking the largest dL
  // also melting the initial config
  double Tmax = 0;
  for (int i=0; i<ncity*N;i++) {
    double dL = UpdatePath(ncity, cities, r);
    if (dL>Tmax) {Tmax=dL;}
  }

  // uncomment to print Tmax found and list of cities
  // printf("Tmax: %.2f km\n",	Tmax);
  // for (int i=0; i<ncity; i++)
  //   printf("%lf %lf\n",	cities[i].lon,cities[i].lat);


  // now do annealing, stopping only when the decrease in path is negligible
  // Create vectors to store the history
  std::vector<double> history_T;
  std::vector<double> history_Dist;
  double prev_dist = distance;
  double dist_change = 1.0;
  double T = Tmax;
  while (dist_change>limit) {
    // keep applying changes to the order until we hit the ncity*N changes at this T
    double nchanges = 0;
    while (nchanges<ncity*N*1.) {
      nchanges += UpdatePath(ncity,cities,r,T);
    }

    // calculate the final distance for this T
    double currL = GetTotalDistance(ncity, cities); // current distance
    dist_change = abs(currL - prev_dist)/prev_dist;
    prev_dist = currL;

    // store annealing history
    history_T.push_back(T);
    history_Dist.push_back(currL);

    // update T
    T = alpha*T;
  }

  // stop timer after annealing is finished
  timer.Stop();


  // get the final total distance and print
  double distance_final = GetTotalDistance(ncity,cities);
  printf("Total ending trip distance: %.2f km\n",	distance_final);
  printf("Distance reduction: %.2f km\n",	distance-distance_final);

  // print the time it took
  timer.Print(); // Prints Real time (wall clock) and Cpu time

  // GENERATE THE PLOT
  PlotCoolingHistory(history_T, history_Dist, filename);
  
  // write resulting path to file
  string outfile = filename.erase(filename.find(".dat"),4) + "final.dat";
  WriteData(outfile, ncity, cities, distance, distance_final);

  return 0;
}
