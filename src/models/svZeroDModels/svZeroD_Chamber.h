#ifndef SVZERODCHAM
#define SVZERODCHAM

# include "svZeroDModel.h"

using namespace std;

// DEFINE CONVERSION CONSTANT
const double pConv = 1.334E3;
const double qConv = 1E0;

class svZeroD_Chamber: public svZeroDModel {
   
  public:
    
    //DEFAULT CONSTRUCTOR
    svZeroD_Chamber(std::string input_target_file, std::string input_perufusion_volumes_file);

    // SET UP MODEL PARAMETERS
    virtual void setupModel(LPNSolverInterface& interface);

    // GET NUMBER OF PARAMETERS
    virtual int getParameterTotal();

    // GET NUMBER OF PARAMETERS (UKNOWNS)
    virtual int getStateTotal();

    // GET TOTAL NUMBER OF RESULTS
    virtual int getResultTotal();

    // GET THE PARAMETER NAMES
    virtual string getParamName(int index);

    // GET THE RESULT NAMES
    virtual string getResultName(int index);

    // GET THE PARAMETER RANGES FOR THE LPN MODELS
    virtual void getParameterLimits(stdVec& limits);

    // GET PARAMETER SETS FOR THE LPN MODELS     
    virtual void getDefaultParams(stdVec& zp);

    // UPDATE ZEROD MODEL PARAMETERS
    virtual void setModelParams(LPNSolverInterface& interface, const stdVec& params);

    // POSTPROCESS ZEROD SIMULATION
    virtual void postProcess(LPNSolverInterface& interface, const stdVec& t, const stdMat& outVals,const stdMat& auxOutVals, stdVec& results);

    // CUSTOM ERROR EVALUATION FOR SPECIFIC MODELS
    virtual double evalModelError(std::vector<double>& results);

    // PRINT OUT RESULTS
    void printResults(int totalResults, double *Xn);

    // READ TARGET FLOWS FROM A FILE
    void readTargetsFromFile(string targetFileName);

    // READ PERFUSION DATA FROM A FILE
    void readPerfusionFile(string perfusionFileName);

    // GET THE DEFAULT PARAMETER RANGES FOR THE LPN MODELS (NOT USED)
    virtual void getDefaultParameterLimits(stdVec& limits);

    virtual void getPriorMapping(int priorModelType,int* prPtr);

    // RETURN THE NUMBER OF EXTRA OUTPUTS
    virtual int  getAuxStateTotal();

    // KEY/NAME FOR EACH TARGET QUANTITY
    virtual void getResultKeys(vector<string>& keys);

    // STANDARD DEVIATION OF EACH TARGET MEASUREMENT
    virtual void getDataStd(stdVec& stdFactors);

    // INVERSE WEIGHT OF EACH TARGET QUANTITY IN LOG LIKELIHOOD
    virtual void getResultWeights(stdVec& weights);

    // RETURN PARAMETER SPECIFIED BY STRING SPECIFIER
    virtual void getSpecifiedParameter(string& specifier, double& return_db_param, int& return_int_param);

  protected:

    int num_blocks;
    std::vector<double> init_state_y, init_state_ydot;
    bool use_perfusion = false;
    std::map<std::string,double> perfusion_data;
    std::vector<int> P_ids;
    std::vector<std::string> P_ids_names;
    std::vector<std::string> outlet_names;
    
    std::vector<double> target_vals;
    std::vector<std::string> target_name;
    
    std::vector<double> read_chamber;
    std::vector<double> read_BC;
    std::vector<double> read_Vessel;
    std::vector<double> all_base;

    int numCycles;
    int total3DSteps;
    int total0DSteps;
    // Number of unknowns
    int nUnknowns;
    //int nBlocks;
    std::vector<std::string> names;
};

#endif // SVZERODCHAM