# =======================
# SELECT TESTS TO INCLUDE
# =======================
OPTION(test_runModels "Run LPN Models with default parameter set" ON)

# ==========================
# SET TARGET LOCATION TO BIN
# ==========================
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==========
# RUN MODELS
# ==========
IF(test_runModels)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/01_runLPN)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/02_optLPN)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/03_useBasis)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/04_optRCR)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/05_runRCR)
  IF(addModelBertini)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/06_nonLinear)
  ENDIF()
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/07_optSvZeroD)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/08_optSvZeroD_distalR)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/09_estimateDREAM)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/10_estimateDREAM_svZeroD)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/11_GSI_svZeroD)
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/test/cppTests/practice_example)
ENDIF()
