#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <octomap/octomap.h>
#include <Eigen/Geometry>
#include <boost/format.hpp>

int main(int argc, char** argv) {

    std::vector<cv::Mat> colorImgs, depthImgs;
    std::vector<Eigen::Isometry3d> poses;

    std::ifstream fin("../data/pose.txt");
    if (!fin) {
        std::cerr << "Cannot find pose file." << std::endl;
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        boost::format fmt("../data/%s/%d.%s");
        colorImgs.push_back(cv::imread((fmt % "color" % (i + 1) % "png").str()));
        depthImgs.push_back(cv::imread((fmt % "depth" % (i + 1) % "png").str(), -1));

        double data[7] = {0};
        for (int i = 0; i < 7; ++i) {
            fin >> data[i];
        }
        Eigen::Quaterniond q(data[6], data[3], data[4], data[5]);
        Eigen::Isometry3d T(q);
        T.pretranslate(Eigen::Vector3d(data[0], data[1], data[2]));
        poses.push_back(T);
    }

    // compute point cloud
    // camera intrinsics
    double cx = 319.5;
    double cy = 239.5;
    double fx = 481.2;
    double fy = -480.0;
    double depthScale = 5000.0;

    std::cout << "Converting image to pointcloud ..." << std::endl;

    // octomap tree
    octomap::OcTree tree(0.01);

    for (int i = 0; i < 5; ++i) {
        std::cout << "Processing image ..." << i + 1 << std::endl;
        cv::Mat color = colorImgs[i];
        cv::Mat depth = depthImgs[i];
        Eigen::Isometry3d T = poses[i];

        octomap::Pointcloud cloud;

        for (int v = 0; v < color.rows; ++v)
            for (int u = 0; u < color.cols; ++u) {
                unsigned int d = depth.ptr<unsigned short >(v)[u];
                if (d == 0) continue;
                Eigen::Vector3d point;
                point[2] = double(d) / depthScale;
                point[0] = (u - cx) * point[2] / fx;
                point[1] = (v - cy) * point[2] / fy;
                Eigen::Vector3d pointWorld = T * point;

                cloud.push_back(pointWorld[0], pointWorld[1], pointWorld[2]);
            }
        tree.insertPointCloud(cloud, octomap::point3d(T(0, 3), T(1, 3), T(2, 3)));
    }

    tree.updateInnerOccupancy();
    std::cout << "saving octomap ..." << std::endl;
    tree.writeBinary("../results/octomap.bt");

    return 0;
}