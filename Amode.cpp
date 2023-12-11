#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>
using namespace std;

//createDataMatrix is used to create a data matrix to hold the ultrasound data
float **createDataMatrix(int numElement, int numSample)
{

    //For createDataMatrix, create a double pointer of float type named RFData
    float **RFData;

    //Allocate a float pointer with numElement elements to RFData using the new keyword, and for each
    //of the float pointer, allocate a new float array of length numSample; 
    //creates a numElement×numSample matrix (2D data matrix)
    RFData  = new float*[numElement];
    for (int i = 0; i < numElement; i++) 
    {    
        RFData[i] = new float[numSample];
    }
    //return RFData (a double pointer of float type)
    return RFData;

}

//loadRFData loads ultrasound data from the file
int loadRFData(float **RFData, const char *fileName, int numElement, int numSample)
{
    // Open the text file fileName, read the data and store into RFData
    // Open the file with the file name stored in filename using ifstream
    ifstream infile(fileName);

    //If the function fails to open the file, cout error message and return -1
    if (!infile) {
        cerr << "Function failed to open the file." << endl;
        return -1; // return an error code
    }

    //Creating a text array to take in data entries from the txt file
    string textArr;

    //Reading the data from the text file and populating the RFData matrix
    //Using for loops to populate each location in the matrix
        for (int i = 0; i < numElement; i++)
        {
            for (int j = 0; j < numSample; j++)
            {
                //using the getline function to read text from the file
                getline(infile, textArr);
                istringstream ss(textArr);
                ss >> RFData[i][j];
            } 
        }
        return 0;
}

// Create an array containing the depth location (in z-direction) for each pixel on the scanline
float *genScanlineLocation(int &numPixel)
{
    //Prompting the user for the desired imaging depth, storing it in scanlineDepth 
    float scanlineDepth;
    cout<< "Depth of scanline: ";
    cin >> scanlineDepth;
    //Prompting the user for the number of pixels, storing it in numPixel 
    cout << "Number of pixels: ";
    cin >> numPixel;

    //Creating an array called scanlineLocation with numPixel elements
    float *scanlineLocation = new float[numPixel];

    //Determine how much to increment each value in order to ensure equal spacing between elements
    float incr = scanlineDepth/(numPixel-1);

    //The number of array elements is equal to number of pixels,
    //and the first element is the starting depth (at 0m).
    //and last element is the imaging depth
    for (int i = 0; i < numPixel; i++)
    {
        scanlineLocation[i] = incr*i;
    }
    
    return scanlineLocation;
}

//This function determines the transducer element locations
float *genElementLocation(int numElement, float PITCH)
{
// Create an array containing the element location (in x-direction) of the ultrasound transducer
   float *eleLocation = new float [numElement];

//For the nth element of an N-element array transducer, determine the physical location 
//(in the x-direction) using the given equation
    for (int n = 0; n < numElement; n++)
    {
        eleLocation[n] = (n - ((numElement-1)/2.0))*PITCH;
    }
    return eleLocation;
}

//Allocate memory to store the beamformed scanline
float *createScanline(int numPixel)
{
    //creating a float array called scanline to store the beamformed scanline
    float *scanline = new float[numPixel];
    return scanline;
}

// Beamform the A-mode scanline
void beamform(float *scanline, float **realRFData, float **imagRFData, float *scanlinePosition, float *elementPosition, int numElement, int numSample, int numPixel, float FS, float SoS)
{
    float tforward;
    float tbackward;
    float ttotal;
    int sample;
    float Preal, Pimag; 


    for (int i =0; i < numPixel; i++)
    {
        //the forward time of flight for the ith position calculated using the given formula
        tforward = scanlinePosition[i]/SoS;
        //the real and imaginary part of the pulse signal Preal and Pimag begin with values of 0.0
        //(the values will later be calculated by taking the sum of the elements in realRFData and imagRFData respectively)
        Preal = 0.0;
        Pimag = 0.0;

        //Combining the formulas, calculate the scanline iterating through to the k'th element
        for (int k = 0; k < numElement; k++)
        {
            //Using the pythagorean theorem to determine the time it takes for the wave to go 
            //from the ith location back to the kth element of the transducer
            tbackward = sqrt(pow(scanlinePosition[i],2)+pow(elementPosition[k],2))/SoS;
            //Calculate the total time of flight from the transducer to the ith location and to the kth element 
            //by summing the tforward and tbackward times together
            ttotal = tforward + tbackward;
            //Sample is the frequency at which the data was sampled at 
            //Employ the floor operation because sample should be an integer for array indexing
            sample = floor(ttotal*FS);
            //Generate the real part and the imaginary part of the pulse signal Preal and Pimag 
            Preal += realRFData[k][sample];
            Pimag += imagRFData[k][sample];
        }
        //Using the real and imaginary parts of the echo signal, calculate the echo magnitude at the ith scanline location
        float mag = sqrt(pow(Preal,2)+ pow(Pimag,2));
        scanline[i] = mag;
    }
}

// Write the scanline to a csv (comma separated value) file
int outputScanline(const char *fileName, float *scanlinePosition, float *scanline, int numPixel)
{
    ofstream outputFile(fileName);

//Create and open an output file with the file name specified in filename using ofstream
    //Display an error message if the output file fails to open
    if (!outputFile) {
        cerr << "Function failed to open the file." << endl;
        return -1; //Return an error code
    }

//Iterate through each of the scanline locations and scanline elements and store them in the output file
    for (int i=0; i < numPixel; i++)
    {
        outputFile << scanlinePosition[i] << ", " << scanline[i] << endl;
    }
    return 0;
}

// Destroy/release all the allocated memory
void destroyAllArrays(float *scanline, float **realRFData, float **imagRFData, float *scanlinePosition, float *elementPosition, int numElement, int numSample, int numPixel)
{
    delete []scanline;
    delete []scanlinePosition;
    delete []elementPosition;

    //For double pointers (realRFData, imagRFData), the memory allocated for each float pointer must be deleted (each element)
    for (int i = 0; i < numElement; i++)
    {
        delete []realRFData[i];
    }
    for (int i = 0; i < numElement; i++)
    {
        delete []imagRFData[i];
    }

    delete []realRFData;
    delete []imagRFData;
}

