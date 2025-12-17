Starter code and data for traveling salesman problem


Files in this directory:

* datareader.cpp : example code to read in the data files (use Makefile)
* datareader.py  : example code to read in the data files
* cities23.dat : list of coordinates for 23 cities in North America
* cities150.dat : 150 cities in North America
* cities1k.dat : 1207 cities in North America
* cities2k.dat : 2063 cities around the world
* routeplot.py : code to plot the globe and salesman's path<br>
usage:<br>
python routeplot.py cities.dat [cities2.dat] -r [="NA"],"World"'<br>
NA = North America, World = Mercator projection of the whole earth
* earth.C : (just for fun) plotting the globe in ROOT


## Instructions to Build and Run
Run make to build salesman.cpp. salesman takes several arguments at the command line:
* -f : .dat filename
* -n : multiplicative factor for number of trials at a certain temperature (ex: n = 500 -> 500*(N cities) trials)
* -a : factor for producing next temperature (ex: T' = a * T)
* -t : final temperature to end annealing at
* -l : limit in change between path between two temperatures. Will end annealing if percent change is below this limit (0 < limit < 1.0)
<br>You must provide a filename and either -t or -l options. Defaults for -n and -a is 10 and 0.8 respectively.<br>
Once salesman is run, you must run routeplot using the appropriate cities[\#]final.dat file to make the plot of the route.<br>

Ex:<br>
./salesman -f cities150.dat -n 500 -a 0.9 -t 0.1 <- Runs salesman for cities150.dat, n=500, a=0.9, down to T=0.1km. Saves to cities150final.dat and plots annealing schedule to an[\#].png<br>
python routeplot.py cities150final.dat <- Plots path to cities150final.pdf

## Results
cities 150 317298.65 48033.77 0:00:03  - CMD: "./salesman -f cities150.dat -n 10 -a 0.99 -l 0.00001" <br>

cities 1k 732177.74 93987.42 0:10:03 - CMD: "./salesman -f cities1k.dat -n 10 -a 0.99 -l 0.00001" <br>

cities 2k 10187617.64 275462.64 0:11:23 - CMD: "./salesman -f cities2k.dat -n 10 -a 0.99 -l 0.00001" <br>

The code starts by melting the initial config from the file and selects a Tmax from the largest change in path length during this process. Updating the path requires the random reversal of a random slice of cities in the path. During annealing, for each temperature there are n*(N cities) successful changes made to the path according to the Metropolis algorithm. The next temperature is a factor a times the previous temperature. Once Tmin or the minimum change in energy between each temperature is reached, the annealing is done. <br>
I tried a couple sets of different parameters, but the choices above seem to give reasonable results within a reasonable amount of time. A large a requires a very gradual drop in temperature while a smallish n makes sure that annealing doesn't take too long. The final limit 0.00001 was chosen since it seems to give fairly reproducible results.
