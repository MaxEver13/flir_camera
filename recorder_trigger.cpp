/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2022-11-08 17:47:03
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-11-10 13:52:56
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

// Use the following enum and global constant to select whether a software or
// hardware trigger is used.
enum triggerType
{
    SOFTWARE,
    HARDWARE
};

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

// 硬件触发模式
const triggerType chosenTrigger = HARDWARE;
const double exposureTime = 3000.0; // us

// This function configures the camera to use a trigger. First, trigger mode is
// set to off in order to select the trigger source. Once the trigger source
// has been selected, trigger mode is then enabled, which has the camera
// capture only a single image upon the execution of the chosen trigger.
// 为每个相机配置相机触发模式，每次触发捕获一张图像
int ConfigureTrigger(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING TRIGGER ***" << endl << endl;

    cout << "Note that if the application / user software triggers faster than frame time, the trigger may be dropped "
            "/ skipped by the camera."
         << endl
         << "If several frames are needed per trigger, a more reliable alternative for such case, is to use the "
            "multi-frame mode."
         << endl
         << endl;

    if (chosenTrigger == SOFTWARE)
    {
        cout << "Software trigger chosen..." << endl;
    }
    else if (chosenTrigger == HARDWARE)
    {
        cout << "Hardware trigger chosen..." << endl;
    }

    try
    {
        //
        // Ensure trigger mode off
        //
        // *** NOTES ***
        // The trigger must be disabled in order to configure whether the source
        // is software or hardware.
        //
        CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
        if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
        {
            cout << "Unable to disable trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
        {
            cout << "Unable to disable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

        cout << "Trigger mode disabled..." << endl;

        //
        // Set TriggerSelector to FrameStart
        //
        // *** NOTES ***
        // For this example, the trigger selector should be set to frame start.
        // This is the default for most cameras.
        //
        CEnumerationPtr ptrTriggerSelector = nodeMap.GetNode("TriggerSelector");
        if (!IsAvailable(ptrTriggerSelector) || !IsWritable(ptrTriggerSelector))
        {
            cout << "Unable to set trigger selector (node retrieval). Aborting..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerSelectorFrameStart = ptrTriggerSelector->GetEntryByName("FrameStart");
        if (!IsAvailable(ptrTriggerSelectorFrameStart) || !IsReadable(ptrTriggerSelectorFrameStart))
        {
            cout << "Unable to set trigger selector (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerSelector->SetIntValue(ptrTriggerSelectorFrameStart->GetValue());

        cout << "Trigger selector set to frame start..." << endl;

        //
        // Select trigger source
        //
        // *** NOTES ***
        // The trigger source must be set to hardware or software while trigger
        // mode is off.
        //
        CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
        if (!IsAvailable(ptrTriggerSource) || !IsWritable(ptrTriggerSource))
        {
            cout << "Unable to set trigger mode (node retrieval). Aborting..." << endl;
            return -1;
        }

        if (chosenTrigger == SOFTWARE)
        {
            // Set trigger mode to software
            CEnumEntryPtr ptrTriggerSourceSoftware = ptrTriggerSource->GetEntryByName("Software");
            if (!IsAvailable(ptrTriggerSourceSoftware) || !IsReadable(ptrTriggerSourceSoftware))
            {
                cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
                return -1;
            }

            ptrTriggerSource->SetIntValue(ptrTriggerSourceSoftware->GetValue());

            cout << "Trigger source set to software..." << endl;
        }
        else if (chosenTrigger == HARDWARE)
        {
            // Set trigger mode to hardware ('Line0')
            CEnumEntryPtr ptrTriggerSourceHardware = ptrTriggerSource->GetEntryByName("Line0");
            if (!IsAvailable(ptrTriggerSourceHardware) || !IsReadable(ptrTriggerSourceHardware))
            {
                cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
                return -1;
            }

            ptrTriggerSource->SetIntValue(ptrTriggerSourceHardware->GetValue());

            cout << "Trigger source set to hardware..." << endl;
        }

        //
        // Turn trigger mode on
        //
        // *** LATER ***
        // Once the appropriate trigger source has been set, turn trigger mode
        // on in order to retrieve images using the trigger.
        //

        CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
        if (!IsAvailable(ptrTriggerModeOn) || !IsReadable(ptrTriggerModeOn))
        {
            cout << "Unable to enable trigger mode (enum entry retrieval). Aborting..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());

        // NOTE: Blackfly and Flea3 GEV cameras need 1 second delay after trigger mode is turned on

        cout << "Trigger mode turned back on..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}


// This function retrieves a single image using the trigger. In this example,
// only a single image is captured and made available for acquisition - as such,
// attempting to acquire two images for a single trigger execution would cause
// the example to hang. This is different from other examples, whereby a
// constant stream of images are being captured and made available for image
// acquisition.
int GrabNextImageByTrigger(INodeMap& nodeMap, CameraPtr pCam)
{
    int result = 0;

    try
    {
        //
        // Use trigger to capture image
        //
        // *** NOTES ***
        // The software trigger only feigns being executed by the Enter key;
        // what might not be immediately apparent is that there is not a
        // continuous stream of images being captured; in other examples that
        // acquire images, the camera captures a continuous stream of images.
        // When an image is retrieved, it is plucked from the stream.
        //
        if (chosenTrigger == SOFTWARE)
        {
            // Get user input
            cout << "Press the Enter key to initiate software trigger." << endl;
            getchar();

            // Execute software trigger
            CCommandPtr ptrSoftwareTriggerCommand = nodeMap.GetNode("TriggerSoftware");
            if (!IsAvailable(ptrSoftwareTriggerCommand) || !IsWritable(ptrSoftwareTriggerCommand))
            {
                cout << "Unable to execute trigger. Aborting..." << endl;
                return -1;
            }

            ptrSoftwareTriggerCommand->Execute();

            // NOTE: Blackfly and Flea3 GEV cameras need 2 second delay after software trigger
        }
        else if (chosenTrigger == HARDWARE)
        {
            // Execute hardware trigger
            cout << "Use the hardware to trigger image acquisition." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function returns the camera to a normal state by turning off trigger
// mode.
int ResetTrigger(INodeMap& nodeMap)
{
    int result = 0;

    try
    {
        //
        // Turn trigger mode back off
        //
        // *** NOTES ***
        // Once all images have been captured, turn trigger mode back off to
        // restore the camera to a clean state.
        //
        CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
        if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
        {
            cout << "Unable to disable trigger mode (node retrieval). Non-fatal error..." << endl;
            return -1;
        }

        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
        {
            cout << "Unable to disable trigger mode (enum entry retrieval). Non-fatal error..." << endl;
            return -1;
        }

        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

        cout << "Trigger mode disabled..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures a custom exposure time. Automatic exposure is turned
// off in order to allow for the customization, and then the custom setting is
// applied.
int ConfigureExposure(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING EXPOSURE ***" << endl << endl;

    try
    {
        //
        // Turn off automatic exposure mode
        //
        // *** NOTES ***
        // Automatic exposure prevents the manual configuration of exposure
        // time and needs to be turned off.
        //
        // *** LATER ***
        // Exposure time can be set automatically or manually as needed. This
        // example turns automatic exposure off to set it manually and back
        // on in order to return the camera to its default state.
        //
        CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");
        if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto))
        {
            cout << "Unable to disable automatic exposure (node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrExposureAutoOff = ptrExposureAuto->GetEntryByName("Off");
        if (!IsAvailable(ptrExposureAutoOff) || !IsReadable(ptrExposureAutoOff))
        {
            cout << "Unable to disable automatic exposure (enum entry retrieval). Aborting..." << endl << endl;
            return -1;
        }

        ptrExposureAuto->SetIntValue(ptrExposureAutoOff->GetValue());

        cout << "Automatic exposure disabled..." << endl;

        //
        // Set exposure time manually; exposure time recorded in microseconds
        //
        // *** NOTES ***
        // The node is checked for availability and writability prior to the
        // setting of the node. Further, it is ensured that the desired exposure
        // time does not exceed the maximum. Exposure time is counted in
        // microseconds. This information can be found out either by
        // retrieving the unit with the GetUnit() method or by checking SpinView.
        //
        CFloatPtr ptrExposureTime = nodeMap.GetNode("ExposureTime");
        if (!IsAvailable(ptrExposureTime) || !IsWritable(ptrExposureTime))
        {
            cout << "Unable to set exposure time. Aborting..." << endl << endl;
            return -1;
        }

        // Ensure desired exposure time does not exceed the maximum
        const double exposureTimeMax = ptrExposureTime->GetMax();
        double exposureTimeToSet = exposureTime;

        if (exposureTimeToSet > exposureTimeMax)
        {
            exposureTimeToSet = exposureTimeMax;
        }

        ptrExposureTime->SetValue(exposureTimeToSet);

        cout << std::fixed << "Exposure time set to " << exposureTimeToSet << " us..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function returns the camera to its default state by re-enabling automatic
// exposure.
int ResetExposure(INodeMap& nodeMap)
{
    int result = 0;

    try
    {
        //
        // Turn automatic exposure back on
        //
        // *** NOTES ***
        // Automatic exposure is turned on in order to return the camera to its
        // default state.
        //
        CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");
        if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto))
        {
            cout << "Unable to enable automatic exposure (node retrieval). Non-fatal error..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrExposureAutoContinuous = ptrExposureAuto->GetEntryByName("Continuous");
        if (!IsAvailable(ptrExposureAutoContinuous) || !IsReadable(ptrExposureAutoContinuous))
        {
            cout << "Unable to enable automatic exposure (enum entry retrieval). Non-fatal error..." << endl << endl;
            return -1;
        }

        ptrExposureAuto->SetIntValue(ptrExposureAutoContinuous->GetValue());

        cout << "Automatic exposure enabled..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acquires and saves single image from a camera.
void* AcquireImage(void* arg)
{
    CameraPtr pCam = *((CameraPtr*)arg);

    int result = 0;
    
    try
    {
        // Retrieve device serial number for filename
        CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");
        std::string serialNumber = "";
        if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        {
            serialNumber = ptrStringSerial->GetValue();
        }
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
             << "Started acquiring images..." << endl;

        // Begin acquiring images
        pCam->BeginAcquisition();
        cout << "[" << serialNumber << "] "
             << "Started acquiring images..." << endl;

        // Retrieve GenICam nodemap
        INodeMap& nodeMap = pCam->GetNodeMap();

        //
        // Retrieve, convert, and save single image for each camera
        //

        try
        {
            // Retrieve the next image from the trigger
            result = result | GrabNextImageByTrigger(nodeMap, pCam);
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
                filename << ".jpg";
                // Save image
                convertedImage->Save(filename.str().c_str());
                // Print image information
                cout << "[" << serialNumber << "] "
                        << "Grabbed image width = " << pResultImage->GetWidth()
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

        // End acquisition
        pCam->EndAcquisition();
        // // Deinitialize camera
        // pCam->DeInit();

        // Reset trigger
        result = result | ResetTrigger(nodeMap);

        // Reset exposure
        result = result | ResetExposure(nodeMap);

        return (void*)1;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;

        // Retrieve GenICam nodemap
        INodeMap& nodeMap = pCam->GetNodeMap();
        
        // Reset trigger
        result = result | ResetTrigger(nodeMap);

        // Reset exposure
        result = result | ResetExposure(nodeMap);

        return (void*)0;
    }
}
// This function acts as the body of the example
int RunMultipleCameras(CameraList camList)
{
    int result = 0;
    int err = 0;
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

        // 初始化相机，配置相机触发模式
        for (unsigned int i = 0; i < camListSize; i++)
        {
            // 选中相机
            CameraPtr pCam = camList.GetByIndex(i);

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
            // 打印设备信息
            PrintDeviceInfo(nodeMapTLDevice, serialNumber);

            // 初始化相机
            pCam->Init();

            // Retrieve GenICam nodemap
            INodeMap& nodeMap = pCam->GetNodeMap();

            // 配置触发模式
            err = ConfigureTrigger(nodeMap);
            if (err < 0)
            {
                cout << "Configure trigger failed" << endl;
                return err;
            }

            // 配置固定曝光时间
            err = ConfigureExposure(nodeMap);
            if (err < 0)
            {
                cout << "Configure exposure failed" << endl;
                return err;
            }
        }

        // 为每个相机创建一个线程获取图像
        for (unsigned int i = 0; i < camListSize; i++)
        {
            // Select camera
            pCamList[i] = camList.GetByIndex(i);
            // Start grab thread
            int err = pthread_create(&(grabThreads[i]), nullptr, &AcquireImage, &pCamList[i]);
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
            // Deinitialize camera
            pCamList[i]->DeInit();
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

// 多个相机配置成硬件触发模式，同步硬件触发采集图像，每个相机每次触发只获取一张图像
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