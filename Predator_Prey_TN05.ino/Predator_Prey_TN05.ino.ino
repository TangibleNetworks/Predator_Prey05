// Predator Prey model for TN_05
// Tangible Networks
//
// There are three types of species. 
// 1. Primary producer (DIP1=OFF , DIP2=OFF)
// 2. Herbivore (DIP1=ON)
// 3. Predator (DIP1=OFF , DIP2=ON)
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

#include <math.h>
#include<TN05.h>                  

// Model parameters
float dt = 0.05;                    // size of time step for Euler method
float population = 0;               // initial populations are exactly zero
float old_population = 0;
float growth_rates[] = {2, -1, -1};          // these are the values for the species intrinsic growth rates (intrinsically prey grow, predators die).
float couplings[] = {0.5,0.5,0.5};          // this defines strenght of species interactions (impact one species has on another)

         
int numberOfPrey = 0;
int connections[] = {0, 0, 0};         // to store if inputs are connected, to avoid multiple calls handshake function
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

  delay(10);                    
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Predator-Prey functions.
////For use in N species simulation. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handshake(){

  for (int i=0; i<6; i++) {
    connections[i] = Tn.isConnected(i);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkType(){

    if (Tn.dip1()){
      type = 2;
    }
    else if (Tn.dip2()){
      type = 3;
    }
    else{
      type = 1;
    }
   
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readInputs(){
  
  numberOfPrey=0;
  for (int i=0; i<6; i++) {
    if (connections[i]){
      if (type == 3){
        inputs[i] = abs(Tn.analogRead(i));  // top predators always receive food!
        numberOfPrey ++;
      }
      else{
        inputs[i] = Tn.analogRead(i);  // everything else just receieves what it is sent.
        if (inputs[i] > 0.0){
          numberOfPrey ++;
        }
      }
      
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
        population += dt * inputs[i] * old_population * couplings[type]/(numberOfPrey+1);
      }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendPopulation(){
 
    if (type==1){
      Tn.analogWrite(population);                 // always send positive
    }
    else {
      Tn.analogWrite(-population/numberOfPrey);  // always send negative (updated to improve stability)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
