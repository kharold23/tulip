#include "acActionOPT_NM.h"
#include "cmLPN_svZeroD.h"
#include "svZeroD_R_RCR.h"

using namespace std;

// =============
// MAIN FUNCTION
// =============
int main(int argc, char* argv[]){

  // Parse command-line inputs
  std::string model_path;
  std::string targets_file;
  std::string perfusion_volumes_file = "None";

  /*
  std::string perfusion_volumes_file;
  if(argc < 3 || argc > 4){
    printf("Usage: optSvZeroD_distalR model_path target_flows (optional: perfusion_volumes)\n");
    printf("Terminated!\n");
    exit(1);
  }else{
  */

  try{
    // Get svZeroD model json file and perfusion volumes file (if provided)
    model_path = string(argv[1]);
    targets_file = string(argv[2]);
    /*
    if(argc == 4) {
      perfusion_volumes_file = string(argv[3]);
    } else {
      perfusion_volumes_file = "None";
    }
    */
  }catch(exception &e){
    // ERROR: TERMINATED!
    printf("Invalid Input Arguments\n");
    printf("TERMINATED!\n");
    return 0;
  }
  //}
  
  // Create new LPN model
  // svZeroDPlus interface library
  auto interface_lib = std::string("/scratch/users/kharold/svZeroDPlus/build/src/interface/libsvzero_interface.so");
  // Type of svZeroD model
  svZeroDModel* zeroDmodel = new svZeroD_R_RCR(targets_file, perfusion_volumes_file);
  cmLPN_svZeroD* lpnModel = new cmLPN_svZeroD(model_path, zeroDmodel, interface_lib, true);

  // Total Number of iterations
  int totIterations = 50;
  // Convergence Tolerance
  double convTol = 1.0e-1;
  // Check Convergence every convUpdateIt iterations
  int convUpdateIt = 10;
  // Maximum Iterations
  int maxOptIt = 200;
  // Coefficient for Step increments
  double stepCoefficient = 0.1;    
  // File with initial starting point
  bool useStartingParameterFromFile = false;
  string startParameterFile("");
  bool startFromCentre = false;

  // INIT ACTION
  acAction* nm = new acActionOPT_NM(convTol, 
                                    convUpdateIt,
                                    maxOptIt,
                                    stepCoefficient);
  
  // SET THE MODEL
  nm->setModel(lpnModel);

  // SET INITIAL GUESS
  ((acActionOPT_NM*)nm)->setInitialParamGuess(useStartingParameterFromFile,
                                                startFromCentre,
                                                startParameterFile);        

  try{

    for(int loopA=0;loopA<totIterations;loopA++){

      //std::cout << "[main] loopA " << loopA << std::endl;
      
      // PERFORM ACTION
      nm->go();

      if(loopA == 0){
        ((acActionOPT_NM*)nm)->setInitialPointFromFile(true);
        ((acActionOPT_NM*)nm)->setInitialPointFile(string("optParams.txt"));
      }

    }


  }catch(exception &e){
    // ERROR: TERMINATED!
    printf("\n");
    printf("TERMINATED!\n");
    return 0;
  }

  // COMPLETED
  std::ofstream fin_file("NM_fin.txt");
  fin_file << "COMPLETE" << std::endl;
  fin_file.close();

  printf("\n");
  printf("COMPLETED!\n");
  return 0;
}
