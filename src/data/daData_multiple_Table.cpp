# include "daData_multiple_Table.h"

using namespace std;

daData_multiple_Table::daData_multiple_Table(bool useSingleColumn,int columnID){
  this->useSingleColumn = useSingleColumn;
  this->columnID = columnID;
}

daData_multiple_Table::~daData_multiple_Table(){

}

void daData_multiple_Table::getIndexSet(stdIntVec& indexSet){
  if(useSingleColumn){
    // Only Specified Column
    indexSet.clear();
    indexSet.push_back(columnID);
  }else{
    // Insert All the Columns
    indexSet.clear();
    int numCols = dict.begin()->second.size();
    for(int loopA=0;loopA<numCols;loopA++){
      indexSet.push_back(loopA);
    }
  }
}

void daData_multiple_Table::readFromFile(string fileName){
  stdStringMat stringTable;
  stdStringVec temp;
  int error = readCSStringTableFromFile(fileName,stringTable);
  if(error != 0){
    throw daException("Error: in Data.readFromFile, invalid file.\n");
  }
  for(int loopA=0;loopA<stringTable.size();loopA++){
  	temp.clear();
  	for(int loopB=1;loopB<stringTable[0].size();loopB++){
      // Float Values
      // printf("Current String: %s\n",stringTable[loopA][loopB].c_str());
  	  temp.push_back(stringTable[loopA][loopB]);
  	}
    // printf("String: %s\n",stringTable[loopA][0].c_str());
  	dict[stringTable[loopA][0]] = temp;
  } 
}

double daData_multiple_Table::evalOBJ(stdStringVec keys,stdVec values,stdVec weights){
  
  // Check The Size of keys and values
  if((keys.size() != values.size())||(keys.size() != weights.size())){
    throw daException("Error in evalOBJ size of keys and values are not consistent.\n");
  }
  if(useSingleColumn){
    if(dict.begin()->second.size() < columnID){
      throw daException("Error in evalOBJ invalid data index.\n");
    }
  }

  // Get Index Set
  stdIntVec indexSet;
  getIndexSet(indexSet);
  
  // Simple Cost Objective: Sum of Percent Change
  double result = 0.0;
  double perc = 0.0;
  int count = 0;
  double computed = 0.0;
  string measuredString;
  double measured = 0.0;
  int dataIndex = 0;

  for(int loopIndex=0;loopIndex<indexSet.size();loopIndex++){
    dataIndex = indexSet[loopIndex];
    for(int loopA=0;loopA<keys.size();loopA++){
      if(dict.find(keys[loopA]) != dict.end()){
        // Found Key
        computed = values[loopA];      
        measuredString = dict[keys[loopA]][dataIndex];
        if(measuredString.compare("none") != 0){
          try{
            measured = atof(measuredString.c_str());
            perc = (100.0*fabs(computed - measured)/(double)measured) * weights[loopA];
            result += perc * perc;
            count++;
          }catch(...){
          }
        }
      }
    }
  }
  result /= (double)count;
  return result;
}

double daData_multiple_Table::evalLogLikelihood(stdStringVec keys,stdVec avValues,stdVec stdFactors,stdVec weights){
  
  // Check The Size of keys and values
  if((keys.size() != avValues.size())||(keys.size() != stdFactors.size())||(keys.size() != weights.size())){
    throw daException("Error in evalPosterior size of keys and values are not consistent.\n");
  }
  if(useSingleColumn){
    if(dict.begin()->second.size() < columnID){
      throw daException("Error in evalPosterior invalid data index.\n");
    }
  }

  // Get Index Set
  stdIntVec indexSet;
  getIndexSet(indexSet);

  //printf("INDEX SET PRINTOUT\n");
  //for(int loopA=0;loopA<indexSet.size();loopA++){
    //printf("%d\n",indexSet[loopA]);
  //}
  
  // Eval log-likelihood
  double result = 0.0;
  double perc = 0.0;
  int    count = 0;
  double computed = 0.0;
  string measuredString;
  double measured = 0.0;
  double stdDev = 0.0;
  double weightVal = 0.0;
  double stdFactor = 0.0;
  int dataIndex = 0;
  
  for(int loopIndex=0;loopIndex<indexSet.size();loopIndex++){
    dataIndex = indexSet[loopIndex];
    for(int loopA=0;loopA<keys.size();loopA++){
      if(dict.find(keys[loopA]) != dict.end()){
        // Found Key
        computed = avValues[loopA];  
        stdFactor = stdFactors[loopA];
        weightVal = weights[loopA];
        measuredString = dict[keys[loopA]][dataIndex];
        if(measuredString.compare("none") != 0){
          try{
            measured = atof(measuredString.c_str());
            stdDev = fabs(stdFactor);          
            perc = 0.5*(computed - measured)*(computed - measured)/((stdDev * stdDev) * weightVal);
            // printf("%f\n",perc);          
            result += perc;
            count++;
          }catch(...){
          }
        }
      }
    }

  }
  // printf("\n");
  // printf("Result: %f\n",result);
  // Return Value
  return result;
}

void daData_multiple_Table::printAndCompare(stdStringVec keys,stdVec values,stdVec weigths){
  
  // Check The Size of keys and values
  if((keys.size() != values.size())||(keys.size() != weigths.size())){
    throw daException("Error in printAndCompare size of keys and values are not consistent.\n");
  }

  // PRINT MAP
  //typedef map<string,stdStringVec>::iterator it_type;
  //for(it_type iterator = dict.begin(); iterator != dict.end(); iterator++) {
  //  printf("Key %s, Value %s\n",iterator->first.c_str(),iterator->second[1].c_str());
  //}

  if(useSingleColumn){
    if(dict.begin()->second.size() < columnID){
      throw daException("Error in printAndCompare invalid data index.\n");
    }
  }

  // Get Index Set
  stdIntVec indexSet;
  getIndexSet(indexSet);

  // Simple Cost Objective: Sum of Percent Change
  FILE* fp;
  fp = fopen("outputTargets.out","w");
  double computed = 0.0;
  string measuredString;
  double measured = 0.0;
  double weight = 0.0;
  int dataIndex = 0;

  for(int loopIndex=0;loopIndex<indexSet.size();loopIndex++){
    
    dataIndex = indexSet[loopIndex];

    fprintf(fp,"%15s %15s %15s %15s\n","Key", "Measured", "Computed", "Weight");
    for(int loopA=0;loopA<keys.size();loopA++){
      if(dict.find(keys[loopA]) != dict.end()){
        // Found Key
        computed = values[loopA];      
        measuredString = dict[keys[loopA]][dataIndex];      
        if(measuredString.compare("none") != 0){        
          try{
            measured = atof(measuredString.c_str());
            weight = weigths[loopA];
            fprintf(fp,"%15s %15.3f %15.3f %15.3f\n",keys[loopA].c_str(),measured,computed,weight);
          }catch(...){
          }
        }
      }
    }
  }
  fprintf(fp,"\n");
  fclose(fp);
}

int daData_multiple_Table::getPatientValue(string key,double &result){
  if(useSingleColumn){
    int retVal = 0;
    if(dict.find(key) != dict.end()){
      try{
        result = atof(dict[key][columnID].c_str());
      }catch(...){
        result = 0.0;
        retVal = 1;
        return retVal;
      }    
      retVal = 0;
    }else{
      result = 0.0;
      retVal = 1;
    }
    return retVal;  
  }else{
    printf("Warning: columnID not specified in daData_multiple_Table::getPatientValue.");
    return 0;
  }
}

// Print dictionary to stdout
void daData_multiple_Table::printToScreen(){

  // Get Index Set
  stdIntVec indexSet;
  getIndexSet(indexSet);

  int dataIndex = 0;
  
  for(int loopIndex=0;loopIndex<indexSet.size();loopIndex++){
    
    dataIndex = indexSet[loopIndex];

    printf("---\n");
    printf("%30s %30s\n","key","value");
    for(dataMap::iterator it = dict.begin(); it!=dict.end();it++){
      printf("%30s %30f\n",it->first.c_str(),atof(it->second[dataIndex].c_str()));
    }
    printf("---\n");
  }
}

// Get Full Data Table
void daData_multiple_Table::getTable(stdMat& table){

  // Get Index Set
  stdIntVec indexSet;
  getIndexSet(indexSet);

  int dataIndex = 0;

  // Loop on the keys  
  table.clear();
  stdVec tmp;
  for(dataMap::iterator it = dict.begin(); it!=dict.end();it++){
    tmp.clear();
    for(int loopIndex=0;loopIndex<indexSet.size();loopIndex++){
      dataIndex = indexSet[loopIndex];
      tmp.push_back(atof(it->second[dataIndex].c_str()));
    }
    table.push_back(tmp);
  }
}
