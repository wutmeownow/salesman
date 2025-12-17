#ifndef SALESMAN_H   // 1. Start Include Guard
#define SALESMAN_H
#define _USE_MATH_DEFINES
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
const int NMAX = 2500;
double dist_matrix[NMAX][NMAX]; // lookup matrix for city separations

// simple structure to store city coordinates
// could also use std::pair<double> 
// or define a class
typedef struct {
  double lon, lat;
  int id; // add an id for a lookup matrix of separations
} COORD;


// helper function to write out data
inline void WriteData(const string& filename, int ncity, const COORD *cities, double d1, double d2) {
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

// fill the array of city locations
inline int GetData(string fname, COORD *cities){
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
    cities[ncity].id=ncity;
    ncity++;
  }
  fclose(fp);
  return ncity;
}

// function to plot annealing schedule
inline void PlotCoolingHistory(const std::vector<double> &v_temp, const std::vector<double> &v_dist, const string& filename) {
    // Create Canvas
    TCanvas *c2 = new TCanvas("c2", "Annealing History", 800, 600);
    
    // Set X-axis to Log Scale (Crucial for Annealing plots)
    c2->SetLogx(); 

    // Create Graph
    // &v_temp[0] gives the pointer to the underlying array of the vector
    TGraph *gr = new TGraph(v_temp.size(), &v_temp[0], &v_dist[0]);

    // Styling
    gr->SetTitle("Annealing History;Temperature (T);Total Distance [km]");
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
    rawName = "an" + rawName.erase(rawName.find("cities"),6);
    c2->SaveAs((rawName + ".png").c_str());

    delete gr;
    delete c2;
}

// calculate separation between two coordinates on globe
inline double GetDistance(const COORD i, const COORD j) {
  // convert to radians
  double ilat = i.lat * M_PI/180.;
  double ilon = i.lon * M_PI/180.;
  double jlat = j.lat * M_PI/180.;
  double jlon = j.lon * M_PI/180.;

  double dlat = (ilat - jlat);
  double dlong = (ilon - jlon);
  double s1 = sin(dlat/2);
  double s2 = sin(dlong/2);
  double a = s1*s1 + cos(ilat)*cos(jlat)*s2*s2;
  double c = 2*atan2(sqrt(a),sqrt(1-a));
  
  return R*c;
}

// get total distance of path around globe
inline double GetTotalDistance(int ncities, const COORD *cities) {
  double dist = 0.;
  for (int i=0;i<ncities;i++) {
    int j = i-1; // index for city before this one
    if (j<0) {j=ncities-1;} // starting city, city before is the last one
    dist += dist_matrix[cities[i].id][cities[j].id];
  }
  return dist;
}

// precompute separations between each city
inline void PrecomputeDistances(int ncity, const COORD *cities) {
    for (int i=0; i<ncity; i++) {
        for (int j=0; j<ncity; j++) {
            // Note: We use the raw loop indices here because 
            // the array hasn't been shuffled yet.
            dist_matrix[i][j] = GetDistance(cities[i], cities[j]);
        }
    }
}

#endif // 2. End Include Guard
