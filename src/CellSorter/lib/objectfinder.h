#ifndef CELLSORTER_OBJECTFINDER_H
#define CELLSORTER_OBJECTFINDER_H

#include <algorithm>
#include <vector>

#include "opencv/cv.hpp"

#include "tracker.h"
#include "matlab_ext.h"
#include "framefinder.h"
#include "experiment.h"

class ObjectFinder {
public:
    ObjectFinder() { m_connectedComponents.setDataFlags(0xffff);}

    int findObjects(Experiment& experiment);

    unsigned long cleanObjects(Experiment& e);

    void setFrame(const cv::Mat& image);

    void setConditions(const Experiment& experiment);

private:
    Tracker m_track;
    std::vector<Tracker> m_trackerList, m_frameTracker;

    framefinder::Frame m_frame;
    int m_cellNum = 0;
    int m_frameNum = 0;
    double m_dist;
    double m_distThreshold;
    int m_countThreshold = 25;
    cv::Point m_centroid;
    DataContainer m_connectedComponents;

    std::vector<std::function<bool (const DataContainer*)>> m_conditions;

    std::pair<double, Tracker> findNearestObject(const cv::Point& object, const std::vector<Tracker>& listOfObjects);
    void writeToDataVector(const bool& newObject, const int& cc_number, Experiment& experiment);

};



#endif //CELLSORTER_OBJECTFINDER_H
