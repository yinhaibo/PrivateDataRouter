del .\SimulatorA\*.log
del .\SImulatorB\*.log
del .\SImulatorC\*.log
del .\MasterA\*.log
del .\MasterB\*.log
del .\MasterC\*.log
cd .\SimulatorA\
start Simulator.exe
cd  ..\MasterA\
start Master.exe
cd  ..\SimulatorB\
start Simulator.exe
cd  ..\MasterB\
start Master.exe
cd  ..\SimulatorC\
start Simulator.exe
cd  ..\MasterC\
start Master.exe