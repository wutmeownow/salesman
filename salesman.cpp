// example code to read in a data file of city lat,long coordinates
#include "salesman.h"



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

  //get ID's of two cities and their partners
  int id_i = cities[iRand].id;
  int id_j = cities[jRand].id;
  int id_i_prev = cities[ioldpartner].id;
  int id_j_next = cities[joldpartner].id;

  // double dold = GetDistance(cities[iRand], cities[ioldpartner]) + GetDistance(cities[jRand], cities[joldpartner]);
  // double dnew = GetDistance(cities[iRand], cities[joldpartner]) + GetDistance(cities[jRand], cities[ioldpartner]);
  // lookup distances from matrix
  double dold = dist_matrix[id_i][id_i_prev] + dist_matrix[id_j][id_j_next];
  double dnew = dist_matrix[id_i][id_j_next] + dist_matrix[id_j][id_i_prev];
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


int main(int argc, char *argv[]){
  TStopwatch timer; // timer to track how long code takes
  timer.Start();
  COORD cities[NMAX];
  TRandom3 *r = new TRandom3(0); // rng

  // parameters
  string filename = ""; // Default value
  double alpha = 0.8; // next T parameter
  int N = 10; // multiplicative factor for number of trials
  double limit = -1.; // limit factor for when to stop annealing
  double Tf = -1.; // final temp for when to stop annealing
  int opt;

  // Loop through all arguments
  // "n:f:" means we expect flags -n and -f, and both require values (:)
  while ((opt = getopt(argc, argv, "f:n:a:l:t:")) != -1) {
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
          case 't':
            Tf = std::stof(optarg);
            break;
          default:
            std::cerr << "Usage: " << argv[0] << " -f <filename> -a <alpha> -n <N> -t <Tmin> -l <limit>" << std::endl;
            return 1;
      }
  }

  if (filename.empty()) {
      std::cerr << "Error: Filename (-f) is required!" << std::endl;
      return 1;
  }

  if (Tf == -1. && limit == -1.) {
    std::cerr << "Error: limit (-l) or Tmin (-t) required!" << std::endl;
    return 1;
  }

  if (Tf != -1. && limit != -1.) {
    std::cerr << "Error: please only provide either limit (-l) or Tmin (-t) to choose when annealing stops!" << std::endl;
    return 1;
  }


  int ncity=GetData(filename,cities);
  printf("Read %d cities from data file\n",ncity);
  // printf("Longitude  Latitude\n");
  // for (int i=0; i<ncity; i++)
  //   printf("%lf %lf\n",	cities[i].lon,cities[i].lat);
  PrecomputeDistances(ncity,cities); // precompute separations and place them in lookup table

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

  // Create vectors to store the history
  std::vector<double> history_T;
  std::vector<double> history_Dist;


  // now do annealing, stopping only when the decrease in path is negligible OR we reach Tf
  double T = Tmax;
  if (Tf!=-1.) {
    // stop annealing when Tf is reached
    while (T>Tf) {
      // keep applying changes to the order until we hit the ncity*N changes at this T
      double nchanges = 0;
      while (nchanges<ncity*N*1.) {
        nchanges += UpdatePath(ncity,cities,r,T);
      }

      // calculate the final distance for this T
      double currL = GetTotalDistance(ncity, cities); // current distance

      // store annealing history
      history_T.push_back(T);
      history_Dist.push_back(currL);

      // update T
      T = alpha*T;
    } 
  } else {
    // stop annealing when change is less than limit
    double prev_dist = distance;
    double dist_change = 1.0;
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
