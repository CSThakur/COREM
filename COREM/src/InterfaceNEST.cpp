#include "InterfaceNEST.h"

InterfaceNEST::InterfaceNEST(void):retina(1,1,1.0),displayMg(1,1),FileReaderObject(1,1,1.0){
    abortExecution = false;
}

InterfaceNEST::InterfaceNEST(const InterfaceNEST& copy){
    abortExecution = false;

}

InterfaceNEST::~InterfaceNEST(void){

}


//------------------------------------------------------------------------------//


void InterfaceNEST::reset(int X, int Y, double tstep,int rep){
    abortExecution = false;
    FileReaderObject.reset(1,1,1.0);
    retina.reset(1,1,1.0);
    displayMg.reset();
}

//------------------------------------------------------------------------------//


double InterfaceNEST::getTotalNumberTrials(){
    return totalNumberTrials;
}

//------------------------------------------------------------------------------//


int InterfaceNEST::getTotalSimTime(){
    return totalSimTime;
}

//------------------------------------------------------------------------------//



double InterfaceNEST::getSimStep(){
    return step;
}

//------------------------------------------------------------------------------//

void  InterfaceNEST::setVerbosity(bool verbose_flag){
    retina.setVerbosity(verbose_flag);
}

bool InterfaceNEST::allocateValues(const char *retinaPath, const char * outputFile,double outputfactor,double currentRep){
    bool ret_correct;

    // Set input directory and parse the retina file
    FileReaderObject.setDir(retinaPath);
    FileReaderObject.allocateValues();
    FileReaderObject.parseFile(retina,displayMg);


    // Set simulation time to 0
    SimTime = 0;

    // Simulation parameters
    totalSimTime = retina.getTotalSimTime();
    totalNumberTrials = retina.getSimTotalTrials();

    CurrentTrial = currentRep;
    retina.setSimCurrentTrial(currentRep);

    if(FileReaderObject.getContReading()){

        // Allocate retina object
        ret_correct = retina.allocateValues();

        // retina size and step
        sizeX=retina.getSizeX();
        sizeY=retina.getSizeY();
        step=retina.getStep();

        // Display manager
        displayMg.setSizeX(sizeX);
        displayMg.setSizeY(sizeY);

        // LN parameters
        displayMg.setLNFile(outputFile,outputfactor);

        // Display manager
        for(int k=0;k<retina.getNumberModules();k++){ // we call addModule() even if retina ony has one module in order to always initialize Displays of Display Manager
            displayMg.addModule(k,(retina.getModule(k))->getModuleID());
        }

    }else {
        abortExecution=true;
        ret_correct=false;
    }
    return(ret_correct);
}



//------------------------------------------------------------------------------//


void InterfaceNEST::update(){
    CImg<double> *input;
    
    input = retina.feedInput(SimTime);
    displayMg.updateDisplay(input, retina, SimTime, totalSimTime, CurrentTrial, totalNumberTrials);
    retina.update(); // This call updates all the modules, so since input is a pointer the content may be modified
    SimTime+=step;
    if(input == NULL) // If this is the end of simulation, end
        abortExecution=true;
}

//------------------------------------------------------------------------------//


double InterfaceNEST::getValue(double cell){

    int select_image = int(cell)/(sizeX*sizeY);
    int pos_im = int(cell) - select_image*(sizeX*sizeY);

    module* aux = retina.getModule(1);
    module* neuron;

    vector <string> layersID;
    layersID = aux->getID(select_image);

    const char * charIDO = (layersID[0]).c_str();
    int neuron_to_display = 0;

    for(int k=0;k<retina.getNumberModules();k++){
        neuron = retina.getModule(k);
        string charID = neuron->getModuleID();
        if (charID.compare(charIDO)==0){
            neuron_to_display = k;
            break;
        }
    }

    neuron = retina.getModule(neuron_to_display);

    int row = pos_im/sizeY;
    int col = pos_im%sizeY;

    return (*neuron->getOutput())(col,row,0,0);


}

//------------------------------------------------------------------------------//

bool InterfaceNEST::getAbortExecution(){
    return abortExecution;
}

//------------------------------------------------------------------------------//

Retina& InterfaceNEST::getRetina(){
    return retina;
}

//------------------------------------------------------------------------------//

void InterfaceNEST::setWhiteNoise(double mean, double contrast1, double contrast2, double period, double switchT,string id,double start, double stop){
    retina.generateWhiteNoise(mean, contrast1, contrast2, period,switchT,sizeX, sizeY);
    displayMg.modifyLN(id,start,stop);
}

void InterfaceNEST::setImpulse(double start, double stop, double amplitude,double offset){
    retina.generateImpulse(start, stop, amplitude,offset, sizeX, sizeY);
}

void InterfaceNEST::setGrating(int type,double step,double lengthB,double length,double length2,int X,int Y,double freq,double T,double Lum,double Cont,double phi,double phi_t,double theta,double red, double green, double blue,double red_phi, double green_phi,double blue_phi){
    retina.generateGrating(type, step, lengthB, length, length2, X, Y, freq, T, Lum, Cont, phi, phi_t, theta,red,green,blue,red_phi, green_phi,blue_phi);
}

