#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <Eigen/Geometry>
#include <boost/format.hpp>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/filters/statistical_outlier_removal.h>

int main(int argc, char** argv) {

    std::vector<cv::Mat> colorImgs, depthImgs;
    std::vector<Eigen::Isometry3d> poses;

    std::ifstream fin("../data/pose.txt");
    if (!fin) {
        std::cerr << "Cannot find pose file." << std::endl;
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        // ../data/color/1.png
        boost::format fmt("../data/%s/%d.%s");
        colorImgs.push_back(cv::imread((fmt % "color" % (i + 1) % "png").str()));
        depthImgs.push_back(cv::imread((fmt % "depth" % (i + 1) % "png").str(), -1)); // -1; original image

        double data[7] = {0};
        for (int i = 0; i < 7; ++i) {
            fin >> data[i];
        }

        // data format
        // tx, ty, tz, qx, qy, qz, qw
        Eigen::Quaterniond q(data[6], data[3], data[4], data[5]);
        Eigen::Isometry3d T(q);
        T.pretranslate(Eigen::Vector3d(data[0], data[1], data[2]));
        poses.push_back(T);
    }

    // compute pointcloud
    // camera intrinsics
    double cx = 319.5;
    double cy = 239.5;
    double fx = 481.2;
    double fy = -480.0;
    double depthScale = 5000.0;

    std::cout << "Converting image to pointcloud ..." << std::endl;

    typedef pcl::PointXYZRGB PointT;
    typedef pcl::PointCloud<PointT> PointCloud;

    // Create a new pointcloud
    PointCloud::Ptr pointCloud(new PointCloud);
    for (int i = 0; i < 5; ++i) {
        PointCloud::Ptr current(new PointCloud);
        std::cout << "Processing image: " << i + 1 << std::endl;
        cv::Mat color = colorImgs[i];
        cv::Mat depth = depthImgs[i];
        Eigen::Isometry3d T = poses[i];
        for (int v = 0; v < color.rows; ++v)
            for (int u = 0; u < color.cols; ++u) {
                unsigned int d = depth.ptr<unsigned short>(v)[u];
                if (d == 0) continue;
                Eigen::Vector3d point;

                /**
                 * u = fx(X/Z) + cx
                 * v = fy(Y/Z) + cy
                 */
                point[2] = double(d) / depthScale; // Z
                point[0] = (u - cx) * point[2] / fx; // X
                point[1] = (v - cy) * point[2] / fy; // Y
                Eigen::Vector3d pointWorld = T * point;

                PointT p;
                p.x = pointWorld[0]; // X
                p.y = pointWorld[1]; // Y
                p.z = pointWorld[2]; // Z
                p.b = color.data[v * color.step + u * color.channels()];
                p.g = color.data[v * color.step + u * color.channels() + 1];
                p.r = color.data[v * color.step + u * color.channels() + 2];
                current->points.push_back(p);
            }

        // depth filter and statistical removal
        PointCloud::Ptr tmp(new PointCloud);
        pcl::StatisticalOutlierRemoval<PointT> statistical_filter;
        statistical_filter.setMeanK(50);
        statistical_filter.setStddevMulThresh(1.0);
        statistical_filter.setInputCloud(current);
        statistical_filter.filter(*tmp);
        (*pointCloud) += *tmp;
    }

    pointCloud->is_dense = false;
    std::cout << "There are " << pointCloud->size() << " points." << std::endl;

    // voxel filter
    pcl::VoxelGrid<PointT> voxel_filter;
    double resolution = 0.03;
    voxel_filter.setLeafSize(resolution, resolution, resolution); // resolution
    PointCloud::Ptr tmp(new PointCloud);
    voxel_filter.setInputCloud(pointCloud);
    voxel_filter.filter(*tmp);
    tmp->swap(*pointCloud);

    std::cout << "After filtering, there are " << pointCloud->size() << " points" << std::endl;

    pcl::io::savePCDFileBinary("../results/map.pcd", *pointCloud);

    return 0;
}