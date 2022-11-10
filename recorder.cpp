/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2022-11-08 17:47:03
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-11-10 11:29:25
 */

#include <iostream>
#include <sstream>
#include <chrono>
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"


using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int PrintDeviceInfo(INodeMap& nodeMap, std::string camSerial)
{
    int result = 0;
    cout << "[" << camSerial << "] Printing device information ..." << endl << endl;
    FeatureList_t features;
    CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
    if (IsAvailable(category) && IsReadable(category))
    {
        category->GetFeatures(features);
        FeatureList_t::const_iterator it;
        for (it = features.begin(); it != features.end(); ++it)
        {
            CNodePtr pfeatureNode = *it;
            CValuePtr pValue = (CValuePtr)pfeatureNode;
            cout << "[" << camSerial << "] " << pfeatureNode->GetName() << " : "
                 << (IsReadable(pValue) ? pValue->ToString() : "Node not readable") << endl;
        }
    }
    else
    {
        cout << "[" << camSerial << "] "
             << "Device control information not available." << endl;
    }
    cout << endl;
    return result;
}

// This function acquires and saves 10 images from a camera.
void* AcquireImages(void* arg)
{
    CameraPtr pCam = *((CameraPtr*)arg);

    try
    {
        // Retrieve TL device nodemap
        INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
        // Retrieve device serial number for filename
        CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");
        std::string serialNumber = "";
        if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        {
            serialNumber = ptrStringSerial->GetValue();
        }
        cout << endl
             << "[" << serialNumber << "] "
             << "*** IMAGE ACQUISITION THREAD STARTING"
             << " ***" << endl
             << endl;
        // Print device information
        PrintDeviceInfo(nodeMapTLDevice, serialNumber);
        // Initialize camera
        pCam->Init();

        // Set acquisition mode to continuous
        CEnumerationPtr ptrAcquisitionMode = pCam->GetNodeMap().GetNode("AcquisitionMode");
        if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
        {
            cout << "Unable to set acquisition mode to continuous (node retrieval; camera " << serialNumber
                 << "). Aborting..." << endl
                 << endl;

            return (void*)0;
        }
        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
        {
            cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval " << serialNumber
                 << "). Aborting..." << endl
                 << endl;

            return (void*)0;
        }
        int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
        cout << "[" << serialNumber << "] "
             << "Acquisition mode set to continuous..." << endl;
        // Begin acquiring images
        pCam->BeginAcquisition();
        cout << "[" << serialNumber << "] "
             << "Started acquiring images..." << endl;
        //
        // Retrieve, convert, and save images for each camera
        //
        const unsigned int k_numImages = 10;
        cout << endl;
        for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            try
            {
                // Retrieve next received image and ensure image completion
                ImagePtr pResultImage = pCam->GetNextImage(1000);
                // Timestamp
                auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::stringstream ss;
                ss << t;
                if (pResultImage->IsIncomplete())
                {
                    cout << "[" << serialNumber << "] "
                         << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl
                         << endl;
                }
                else
                {
                    // Convert image to mono 8
                    ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
                    // Create a unique filename
                    ostringstream filename;
                    filename << ss.str();
                    filename << "-";
                    if (serialNumber != "")
                    {
                        filename << serialNumber.c_str();
                    }
                    filename << "-" << imageCnt << ".jpg";
                    // Save image
                    convertedImage->Save(filename.str().c_str());
                    // Print image information
                    cout << "[" << serialNumber << "] "
                         << "Grabbed image " << imageCnt << ", width = " << pResultImage->GetWidth()
                         << ", height = " << pResultImage->GetHeight() << ". Image saved at " << filename.str() << endl;
                }
                // Release image
                pResultImage->Release();
                cout << endl;
            }
            catch (Spinnaker::Exception& e)
            {
                cout << "[" << serialNumber << "] "
                     << "Error: " << e.what() << endl;
            }
        }
        // End acquisition
        pCam->EndAcquisition();
        // Deinitialize camera
        pCam->DeInit();

        return (void*)1;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;

        return (void*)0;
    }
}
// This function acts as the body of the example
int RunMultipleCameras(CameraList camList)
{
    int result = 0;
    unsigned int camListSize = 0;
    try
    {
        // Retrieve camera list size
        camListSize = camList.GetSize();
        // Create an array of CameraPtrs. This array maintenances smart pointer's reference
        // count when CameraPtr is passed into grab thread as void pointer
        // Create an array of handles
        CameraPtr* pCamList = new CameraPtr[camListSize];

        pthread_t* grabThreads = new pthread_t[camListSize];

        for (unsigned int i = 0; i < camListSize; i++)
        {
            // Select camera
            pCamList[i] = camList.GetByIndex(i);
            // Start grab thread
            int err = pthread_create(&(grabThreads[i]), nullptr, &AcquireImages, &pCamList[i]);
            assert(err == 0);
        }

        for (unsigned int i = 0; i < camListSize; i++)
        {
            // Wait for all threads to finish
            void* exitcode;
            int rc = pthread_join(grabThreads[i], &exitcode);
            if (rc != 0)
            {
                cout << "Handle error from pthread_join returned for camera at index " << i << endl;
                result = -1;
            }
            else if ((int)(intptr_t)exitcode == 0) // check thread return code for each camera
            {
                cout << "Grab thread for camera at index " << i
                     << " exited with errors."
                        "Please check onscreen print outs for error details"
                     << endl;
                result = -1;
            }
        }

        // Clear CameraPtr array and close all handles
        for (unsigned int i = 0; i < camListSize; i++)
        {
            pCamList[i] = 0;
        }
        // Delete array pointer
        delete[] pCamList;
        // Delete array pointer
        delete[] grabThreads;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
    return result;
}

// 多个相机同步采集10张图片
int main(int /*argc*/, char** /*argv*/)
{
    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE* tempFile = fopen("test.txt", "w+");
    if (tempFile == nullptr)
    {
        cout << "Failed to create file in current folder.  Please check permissions." << endl;
        cout << "Press Enter to exit..." << endl;
        getchar();
        return -1;
    }
    fclose(tempFile);
    remove("test.txt");
    int result = 0;
    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;
    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();
    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
    cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
         << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl
         << endl;
    // Retrieve list of cameras from the system
    CameraList camList = system->GetCameras();
    unsigned int numCameras = camList.GetSize();
    cout << "Number of cameras detected: " << numCameras << endl << endl;
    // Finish if there are no cameras
    if (numCameras == 0)
    {
        // Clear camera list before releasing system
        camList.Clear();
        // Release system
        system->ReleaseInstance();
        cout << "Not enough cameras!" << endl;
        cout << "Done! Press Enter to exit..." << endl;
        getchar();
        return -1;
    }
    // Run example on all cameras
    cout << endl << "Running example for all cameras..." << endl;
    result = RunMultipleCameras(camList);
    cout << "Example complete..." << endl << endl;
    // Clear camera list before releasing system
    camList.Clear();
    // Release system
    system->ReleaseInstance();
    cout << endl << "Done! Press Enter to exit..." << endl;
    getchar();
    return result;
}