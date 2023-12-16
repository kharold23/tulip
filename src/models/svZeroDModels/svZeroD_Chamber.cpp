#include "svZeroD_Chamber.h"

using namespace std;

svZeroD_Chamber::svZeroD_Chamber(std::string target_file, std::string perfusion_volumes_file) {
  // Read target flows
  this->readTargetsFromFile(target_file);

}

// ==========================
// SET UP MODEL-SPECIFIC PARAMETERS FOR OPTIMIZATION
// ==========================
void svZeroD_Chamber::setupModel(LPNSolverInterface& interface){
  
  // Load shared library and get interface functions.
  this->nUnknowns = interface.system_size_;
  
  // Number of blocks and number of each type
  this->num_blocks = interface.block_names_.size();
  std::string block_name, branch_name;
  for (int i = 0; i < this->num_blocks; i++) {
    block_name = interface.block_names_[i];
    std::cout << block_name << std::endl;
    this->names.push_back(block_name);
  }

  // Initialize parameter vectors and read baseline block params
  // Baseline parameters (named *_base) are set in the svZeroD config file
  
  // Get parameters for the chamber (Vrd Vrs, L), the BC (R), and the Vessel (C)
  this->read_chamber.resize(7);
  this->read_BC.resize(2);
  this->read_Vessel.resize(4);
  this->all_base.reserve(4);

  for (int i = 0; i < this->num_blocks; i++) {
    block_name = this->names[i];
    if (block_name == "ventricle"){
        interface.read_block_params(this->names[i], this->read_chamber);
    }
    if (block_name == "OUTLET"){
        interface.read_block_params(this->names[i], this->read_BC);
    }
    if (block_name == "aorta"){
        interface.read_block_params(this->names[i], this->read_Vessel);
    }
  }
  this->all_base[0] = this->read_chamber[0];
  this->all_base[1] = this->read_Vessel[0];
  this->all_base[2] = this->read_Vessel[1];
  this->all_base[3] = this->read_BC[0];
  
  // Save solution IDs corresponding to important quantities (Vessel pressure)
  this->P_ids.resize(this->target_vals.size());
  this->P_ids_names.resize(this->target_vals.size());
  string var_name;
  
  // Iterate through the names of variables in the 0D system
  for (int i = 0; i < this->outlet_names.size(); i++) {
    var_name = this->outlet_names[i];
    for (int j = 0; j < interface.system_size_; j++){
        if (var_name == interface.variable_names_[j]){
            this->P_ids[i] = j;
            this->P_ids_names[i] = var_name;
            std::cout<<"Found P_id for "<< P_ids_names[i] << " at " << this->P_ids[i] <<std::endl;
        }
    }
  }
  
  // Check to make sure all variables ids have been assigned
  for (int i = 0; i < 2; i++) {
    if (this->P_ids[i] < 0) {
      std::cout << "P index: "<< i << std::endl;
      std::cout << "Error: Did not find all solution IDs" << std::endl;
      throw std::runtime_error("Error: Did not find all solution IDs");
    }
  }
  
  if (this->target_vals.size() != (this->P_ids.size())) {
    std::cout << "Error: Number of target pressures does not match number of saved pressure state IDs" << std::endl;
    throw std::runtime_error("Error: Number of target pressures does not match number of saved pressure state IDs");
  }

}

// ==========================
// READ TARGET DATA FROM FILE
// ==========================
void svZeroD_Chamber::readTargetsFromFile(string targetFileName)
{
  std::ifstream read_file;
  read_file.open(targetFileName.c_str());
  
  std::string buffer;
  std::vector<std::string> tokens;
  if(!read_file.is_open())
    throw cmException("ERROR: Cannot open targets file");
  if(read_file.eof())
    throw cmException("ERROR: No targets found!");

  while(std::getline(read_file,buffer))
  {
    cmUtils::schSplit(buffer,tokens," ");
    this->target_name.push_back(tokens[1].c_str());
    this->target_vals.push_back(atof(tokens[2].c_str()));
    this->outlet_names.push_back(tokens[0].c_str());
  }
  
  read_file.close();
}

// ==========================
// READ PERFUSION DATA FROM FILE
// ==========================
void svZeroD_Chamber::readPerfusionFile(string perfusionFileName)
{
  std::ifstream read_file;
  read_file.open(perfusionFileName.c_str());
  
  std::string buffer;
  std::vector<std::string> tokens;
  if(!read_file.is_open())
    throw cmException("ERROR: Cannot open perfusion file");
  if(read_file.eof())
    throw cmException("ERROR: No perfusion data found!");
  
  int line_ct = 0;
  while(std::getline(read_file,buffer))
  {
    if (line_ct > 0) { //skip header
      cmUtils::schSplit(buffer,tokens," ");
      this->perfusion_data.insert({tokens[0].c_str(),atof(tokens[1].c_str())});
    }
    line_ct++;
  }
  
  read_file.close();
}

// ========================
// GET NUMBER OF PARAMETERS
// ========================
int svZeroD_Chamber::getParameterTotal(){
  return 4; //Emax, C, Rp, Rd
}

// ===================================
// GET NUMBER OF PARAMETERS (UNKNOWNS)
// ===================================
int svZeroD_Chamber::getStateTotal(){
  return this->nUnknowns; 
}

// ===========================
// GET TOTAL NUMBER OF RESULTS
// ===========================
int svZeroD_Chamber::getResultTotal(){
  return this->target_vals.size();
}

// ==================
// GET PARAMETER NAME
// ==================
string svZeroD_Chamber::getParamName(int index) {
  if (index < this->target_vals.size()) {
    return this->P_ids_names[index];
  } else {
    throw std::runtime_error("ERROR: Invalid index in svZeroD_Chamber::getParamName(index)");
  }
}

// ==================
// GET PARAMETER NAME
// ==================
string svZeroD_Chamber::getResultName(int index) {
  if (index < this->target_vals.size()) {
    return this->P_ids_names[index];
  } 
  else{
      throw std::runtime_error("ERROR: Invalid index in svZeroD_Chamber::getResultName(index)");
  }
}

// ====================
// RETURN PARAMETER SPECIFIED BY STRING SPECIFIER
// ====================
void svZeroD_Chamber::getSpecifiedParameter(string& specifier, double& return_db_param, int& return_int_param) {
  if (specifier == "RScaling") {
    //return_db_param = this->R_scaling;
    std::cout << "RScaling not implemented for this case." << std::endl;
  } else {
    throw std::runtime_error("ERROR: Invalid specifier in svZeroD_Chamber::getSpecifiedParameter.");
  }
}

// ====================
// GET MODEL PARAMETERS
// ====================
void svZeroD_Chamber::getDefaultParams(stdVec& default_params){
      
  default_params.resize(getParameterTotal());

  for (int i = 0; i < getParameterTotal(); i++) {
    default_params[i] = this->all_base[i];
  }

}

// ====================
// GET PARAMETER RANGES
// ====================
void svZeroD_Chamber::getParameterLimits(stdVec& limits){

  limits.resize(2*getParameterTotal());
  //Emax
  limits[0] = 1.0;
  limits[1] = 15.0;
  //Rp
  limits[2] = 0.0001;
  limits[3] = 200.0;
  //Vessel C
  limits[4] = 0.00001;
  limits[5] = 5.0;
  //Rd
  limits[6] = 0.0001;
  limits[7] = 200.0;

}

void svZeroD_Chamber::printResults(int totalResults, double* Xn) {
  printf("RESULT PRINTOUT\n");
  for(int loopA = 0; loopA < totalResults; loopA++) {
    string s = getResultName(loopA);
    printf("%20s : %f\n",s.c_str(),Xn[loopA]);
  }
}


// ==========================================
// UPDATE PARAMETERS OF THE ZEROD MODEL
// ==========================================
void svZeroD_Chamber::setModelParams(LPNSolverInterface& interface, const stdVec& params) {

  std::string block_name;
  
  this->read_chamber[0] = params[0];
  
  this->read_Vessel[0] = params[1];
  this->read_Vessel[1] = params[2];
  
  this->read_BC[0] = params[3];
  
  // Update the model parameters
  for (int i = 0; i < this->num_blocks; i++) {
    block_name = this->names[i];
    if (block_name == "ventricle"){
        interface.update_block_params(this->names[i], this->read_chamber);
    }
    if (block_name == "OUTLET"){
        interface.update_block_params(this->names[i], this->read_BC);
    }
    if (block_name == "aorta"){
        interface.update_block_params(this->names[i], this->read_Vessel);
    }
  }
}

// ==========================================
// POSTPROCESS ZEROD SIMULATION
// ==========================================
void svZeroD_Chamber::postProcess(LPNSolverInterface& interface, const stdVec& t, const stdMat& outVals,const stdMat& auxOutVals, stdVec& results) {
  int totOutputSteps = interface.num_output_steps_;
  int totalStepsOnSingleCycle = interface.pts_per_cycle_;
  
  double value = 0.0;
  double v1;
  double v2;
  
  for(int i = 0; i < P_ids.size(); i++){ 
    if (this->target_name[i] == "min"){
        value = cmUtils::getMin(totOutputSteps - totalStepsOnSingleCycle - 1, totOutputSteps, outVals[this->P_ids[i]]);
    }
    if (this->target_name[i] == "max"){
        value = cmUtils::getMax(totOutputSteps - totalStepsOnSingleCycle - 1, totOutputSteps, outVals[this->P_ids[i]]);
    }
    if (this->target_name[i] == "mean"){
        value = cmUtils::getMean(totOutputSteps - totalStepsOnSingleCycle - 1, totOutputSteps, outVals[this->P_ids[i]]);
    }
    if (this->target_name[i] == "diff"){
        v1 = outVals[this->P_ids[i]][totOutputSteps - totalStepsOnSingleCycle - 1];
        v2 = outVals[this->P_ids[i]][totOutputSteps - 2*totalStepsOnSingleCycle - 1];
        value = 1.0 + v1 - v2;
    }
    results[i] = value;
  }

}


// =========================
// EVAL MODEL ERROR FUNCTION
// =========================
double svZeroD_Chamber::evalModelError(std::vector<double>& results) {
  //std::cout << "[evalModelError] START " << std::endl;

  int totalParams = getParameterTotal();
  int resultTotal = getResultTotal();

  // Compute mean squared percentage error and mean percentage error of flow fractions
  double loss = 0.0;
  double sq_pct_error = 0.0, pct_error = 0.0;
  for(int i=0; i < resultTotal; i++){
    sq_pct_error = (results[i] - this->target_vals[i])*(results[i] - this->target_vals[i])/(this->target_vals[i]*this->target_vals[i]);
    loss += sq_pct_error;
    if (this->target_name[i] == "diff"){
        loss += sq_pct_error*4.0;
    }
    pct_error += sqrt(sq_pct_error);
    std::cout<<"[evalModelError] results, targets: "<<results[i]<<" "<<this->target_vals[i]<<std::endl;
  }
  loss = loss/double(resultTotal); // mean of square pct error
  pct_error = 100.0*pct_error/double(resultTotal); // mean pct error
  std::cout<<"[evalModelError] Mean pct error: "<<pct_error<<std::endl;


  //std::cout << "[evalModelError] END " << std::endl;
  return loss;
}

// ====================
// GET DEFAULT RANGES
// ====================
void svZeroD_Chamber::getDefaultParameterLimits(stdVec& limits) {
  std::cout<<"ERROR: svZeroD_Chamber::getDefaultParameterLimits not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getDefaultParameterLimits not implemented.");
}

// ====================
// GET PRIOR MAPPING
// ====================
void svZeroD_Chamber::getPriorMapping(int priorModelType,int* prPtr) {
  std::cout<<"ERROR: svZeroD_Chamber::getPriorMapping not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getPriorMapping not implemented.");
}

// ====================
// GET NUMBER OF AUXILIARY STATES
// ====================
int svZeroD_Chamber::getAuxStateTotal(){
  std::cout<<"ERROR: svZeroD_Chamber::getAuxStateTotal not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getAuxStateTotal not implemented.");
  return 0;
}

// =========================
// KEY/NAME FOR EACH TARGET QUANTITY
// =========================
void svZeroD_Chamber::getResultKeys(vector<string>& keys) {
  std::cout<<"ERROR: svZeroD_Chamber::getResultKeys not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getResultKeys not implemented.");
}

// =========================
// STANDARD DEVIATION OF EACH TARGET MEASUREMENT
// =========================
void svZeroD_Chamber::getDataStd(stdVec& stdFactors) {
  std::cout<<"ERROR: svZeroD_Chamber::getDataStd not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getDataStd not implemented.");
}

// =========================
// INVERSE WEIGHT OF EACH TARGET QUANTITY IN LOG LIKELIHOOD
// =========================
void svZeroD_Chamber::getResultWeights(stdVec& weights) {
  std::cout<<"ERROR: svZeroD_Chamber::getResultWeights not implemented."<<std::endl;
  std::cout<<"Execution should be terminated but might not if this is in a try-catch block."<<std::endl;
  std::runtime_error("ERROR: svZeroD_Chamber::getResultWeights not implemented.");
}