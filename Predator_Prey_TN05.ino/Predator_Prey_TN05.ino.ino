// Predator Prey model for TN_05
// Tangible Networks
//
// There are three types of species. 
// 1. Primary producer 
// 2. Herbivore 
// 3. Predator 
//
// Their rules for updating their popualtions are as follows:
//
// 1. Primary producers:
//    > send all ouptut as +ve s.t. receiving species know they are being fed
//    > treat all input as negative i.e. being eaten
//
// 2. Herbivores:
//    > send all ouptut as -ve s.t. they compete directly with any other herbivore
//    > determine if they are being eaten or fed based on the sign of their input
//
// 3. Predators:
//    > send all ouptut as -ve s.t. receiving species know they are being eaten
//    > treat all input as positive i.e. being fed
//
// Ports 0-2 are prey ports
// Ports 3-5 are predator ports
//
// NOTE: two possible parameters to tune. 1. The coupling for the top predator (currently 0.4), and 2. the possibly introducing number of prey to interaction dynamics method. 
//   > what population to reset to?
//   > how to tune delay using mater dial

#include <math.h>
#include<TN05.h>                  
# include <stdlib.h>

// Model parameters
float dt = 0.05;                    // size of time step for Euler method
float population = 0;               // initial populations are exactly zero
float old_population = 0;
float growth_rates[] = {2, -1, -1};          // these are the values for the species intrinsic growth rates (intrinsically prey grow, predators die).
float couplings[] = {0.5,0.5,0.4};          // this defines strenght of species interactions (impact one species has on another)

         
int numberOfPrey = 0;
int connections[] = {0, 0, 0, 0, 0, 0};         // to store if inputs are connected, to avoid multiple calls handshake function
float inputs[] = {0,0,0};              // likewise for inputs
int type = 0;  // Species type, as described above.

const float population_max = 20;

TN Tn = TN(-population_max,population_max);   

void setup () {}                

void loop() {

  handshake();                     // check which inputs are connected
  checkType();                     // determine species type based on DIPS
  readInputs();                    // store the inputs locally as they are used by the node to determine its behaviour
  
  old_population = population;  

  updatePopulation();              // do intrinsic and interaction dynamics
  sendPopulation();
  ledWrite();

  delay(10 - Tn.masterRead()*10);

  if (Tn.masterSw()){
    reset_pops();            
  }
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Predator-Prey functions.
////For use in N species simulation. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reset_pops(){
  population = (population_max/(2.0*type)); 
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handshake(){

  for (int i=0; i<6; i++) {
    connections[i] = Tn.isConnected(i);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkType(){
   if (connections[0] + connections[1] + connections[2] == 0){
     type = 3;
   }
   else if (connections[3] + connections[4] + connections[5] == 0){
     type = 1;  
   }
   else{
     type = 2;
   }

//    if (Tn.dip0()){
//      type = 2;
//    }
//    else if (Tn.dip1()){
//      type = 3;
//    }
//    else{
//      type = 1;
//    }
   
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readInputs(){
  
  numberOfPrey=0;
  for (int i=0; i<3; i++) {
    if (connections[i]){
        inputs[i] = -(Tn.analogRead(i));  // prey ports always have negative effect
    }
  }

  for (int i=3; i<6; i++) {
    if (connections[i]){
        inputs[i] = Tn.analogRead(i);  // predator ports always have positive effect
        numberOfPrey ++;
    }
   }

  if (numberOfPrey==0){numberOfPrey++;} 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updatePopulation(){
  
// if switch is not pressed, update population based on Lotka-Volterra dynamics:
  if (!Tn.sw()){

    intrinsicDynamics();  
    interactionDynamics();
  
  }
  // if switch is pressed, increase the population and ignore the LV dynamics:
  else{
    population += 0.02;	
  }

  // bound the population at population_max
  population = MIN(population,population_max);
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void intrinsicDynamics() {
 
  population += growth_rates[type-1]*old_population*dt*(1+10*Tn.pot());
 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void interactionDynamics(){

  for (int i=0; i<6; i++) {
      if (connections[i]){
        population += dt * inputs[i] * old_population * couplings[type-1]; /// (numberOfPrey+1);
      }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendPopulation(){
 
    if (type==1){
      Tn.analogWrite(population);                 
    }
    else {
      Tn.analogWrite(population/numberOfPrey);  
    }
  
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ledWrite(){
  
  if (type==1){    
    Tn.colour(0, round(population*255/population_max), 0);    // basal        -> green
  }
  else if (type==3){
    Tn.colour(round(population*255/population_max), 0, 0);    // top predator -> red
  }
  else if (type==2){
    Tn.colour(round(population*255/population_max), round(population*255/population_max), 0);    // herbivore -> yellow
  }  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;
 
  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }
 
  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;
 
  call = !call;
 
  return (mu + sigma * (double) X1);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
