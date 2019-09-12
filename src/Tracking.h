/**
* This file is part of ORB-SLAM.
*
* Copyright (C) 2014 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <http://webdiis.unizar.es/~raulmur/orbslam/>
*
* ORB-SLAM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRACKING_H
#define TRACKING_H

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "OSLAM.h"
#include "Map.h"
#include "LocalMapping.h"
#include "LoopClosing.h"
#include "Frame.h"
#include "ORBVocabulary.h"
#include "KeyFrameDatabase.h"
#include "ORBextractor.h"
#include "Initializer.h"
#include "GSLAM/core/GSLAM.h"

#if __linux
    #ifndef INCLUDED_GL_H
    #define INCLUDED_GL_H
    #include <GL/glew.h>
    //#include <GL/gl.h>
    #include <GL/glut.h>
    #endif
#else
#include <GL/glew.h>
    //#include <GL/gl.h>
    #include "gui/GL_headers/glext.h"
#endif

inline void glVertex(const GSLAM::Point3d& pt)
{
    glVertex3d(pt.x,pt.y,pt.z);
}

inline void glVertex(const GSLAM::Point3f& pt)
{
    glVertex3f(pt.x,pt.y,pt.z);
}

inline void glColor(const GSLAM::Point3ub& color)
{
    glColor3ub(color.x,color.y,color.z);
}


namespace ORB_SLAM {

class FramePublisher;
class Map;
class LocalMapping;
class LoopClosing;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

///
/// \brief The Tracking class
///
class Tracking
{  

public:
    Tracking(ORBVocabulary* pVoc, Map* pMap);

    enum eTrackingState {
        SYSTEM_NOT_READY=-1,
        NO_IMAGES_YET=0,
        NOT_INITIALIZED=1,
        INITIALIZING=2,
        WORKING=3,
        LOST=4
    };

    void SetLocalMapper(LocalMapping* pLocalMapper);
    void SetLoopClosing(LoopClosing* pLoopClosing);
    void SetKeyFrameDatabase(KeyFrameDatabase* pKFDB);


    void ForceRelocalisation();

    eTrackingState mState;
    eTrackingState mLastProcessedState;    

    // Current Frame
    Frame mCurrentFrame;

    // Initialization Variables
    std::vector<int> mvIniLastMatches;
    std::vector<int> mvIniMatches;
    std::vector<cv::Point2f> mvbPrevMatched;
    std::vector<cv::Point3f> mvIniP3D;
    Frame mInitialFrame;

    void Reset();
    void ResetIfRequested(void);
    void CheckResetByPublishers();

    void track(const cv::Mat &img, double timestamp);
    bool track(GSLAM::FramePtr& videoframe);

    void Draw_Something();
protected:

    void FirstInitialization();
    void Initialize();
    void CreateInitialMap(cv::Mat &Rcw, cv::Mat &tcw);


    bool TrackPreviousFrame();
    bool TrackWithMotionModel();

    bool RelocalisationRequested();
    bool Relocalisation();    

    void UpdateReference();
    void UpdateReferencePoints();
    void UpdateReferenceKeyFrames();

    bool TrackLocalMap();
    void SearchReferencePointsInFrustum();

    bool NeedNewKeyFrame();
    void CreateNewKeyFrame();

    class ScopedLogger
    {
    public:
        ScopedLogger(std::stringstream& sst):_sst(sst),_verbose(svar.GetInt("SLAM.Verbose")){
            _sst.str("");
        }
        ~ScopedLogger(){
            if(_sst.str().size()&&(_verbose&0x01))
                LOG(INFO)<<_sst.str();
        }

        std::stringstream& _sst;
        int                _verbose;
    };
    std::stringstream       _logger;

    //Other Thread Pointers
    LocalMapping* mpLocalMapper;
    LoopClosing* mpLoopClosing;

    //ORB
    ORBextractor* mpORBextractor;
    ORBextractor* mpIniORBextractor;

    //BoW
    ORBVocabulary* mpORBVocabulary;
    KeyFrameDatabase* mpKeyFrameDB;

    // Initalization
    Initializer* mpInitializer;

    //Local Map
    KeyFrame* mpReferenceKF;
    std::vector<KeyFrame*> mvpLocalKeyFrames;
    std::vector<MapPoint*> mvpLocalMapPoints;

    //Map
    Map* mpMap;

    //Calibration matrix
    cv::Mat mK;
    cv::Mat mDistCoef;

    //New KeyFrame rules (according to fps)
    int mMinFrames;
    int mMaxFrames;

    //Current matches in frame
    int mnMatchesInliers;

    //Last Frame, KeyFrame and Relocalisation Info
    KeyFrame* mpLastKeyFrame;
    Frame mLastFrame;
    unsigned int mnLastKeyFrameId;
    unsigned int mnLastRelocFrameId;

    //Mutex
    boost::mutex mMutexTrack;
    boost::mutex mMutexForceRelocalisation;

    //Reset
    bool mbPublisherStopped;
    bool mbReseting;
    boost::mutex mMutexReset;

    //Is relocalisation requested by an external thread? (loop closing)
    bool mbForceRelocalisation;

    //Motion Model
    bool mbMotionModel;
    cv::Mat mVelocity;

    //Color order (true RGB, false BGR, ignored if grayscale)
    bool mbRGB;
};

} //namespace ORB_SLAM

#endif // TRACKING_H
