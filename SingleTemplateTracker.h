#pragma once
/// ���࣬�̳��Ը���Tracker
//#include <iostream>
//#include <string>
//#include <vector>
//#include <opencv2\opencv.hpp>
#include "Tracker.h"

using namespace std;
using namespace cv;

namespace mycv {

	class SingleTemplateTracker : public mycv::Tracker
	{
	public:
		enum MatchMethod { SQDIFF = 0, SADIFF = 1 };
		enum MatchStrategy { UNIFORM = 0, NORMAL = 1 };		//0����ͨ�ķ�����1��������
		
		struct Params
		{
			// ������Χ����չ
			int expandWidth;
			// ƥ�䷽��
			MatchMethod matchMethod;
			// ģ��ƥ�����
			MatchStrategy matchStrategy;
			// ģ������ٶ�
			double alpha;
			// ģ��ƥ��ʱ��������������
			int numPoints;
			// ��������㣨�ض���̬�ֲ����ı�׼��
			Point2d sigma;
			// ģ���ڵĲ�������
			Vec2i xyStep;
			// ģ����ͼ���ڵĻ�������
			Vec2i xyStride;
			Params() {
				expandWidth = 50;
				matchMethod = MatchMethod::SADIFF;
				matchStrategy = MatchStrategy::NORMAL;
				alpha = 0.7;
				numPoints = 500;
				sigma = Point2d(0.5, 0.5);
				xyStep = Vec2i(2, 2);
				xyStride = Vec2i(1, 1);
			}
		};

	public:
		SingleTemplateTracker(Params _params);
		virtual ~SingleTemplateTracker();
		// ��ʼ��������
		bool init(const Mat& initFrame, const Rect& initBoundingBox);
		// ����Ŀ��
		bool track(const Mat& currentFrame, Rect& currentBoundingBox);
		// ����Ŀ��ģ��
		bool update(Rect& NextSearchBox, double& fps);
		// ������ͼ���Ͻ���ģ��ƥ��
		float MatchTemplate(const Mat& src, const Mat& templ, Rect2i& match_location,
			MatchMethod match_method, Vec2i& xy_step, Vec2i& xy_stride);
		// ������һ֡��������Χ
		void EstimateSearchArea(const Rect& target_location, Rect& search_area, int expand_x, int expand_y);
		// ��ָ���Ĳ�����Χ�ڲ����ض���̫�ֲ��㼯
		void GenerateRandomSamplePoints(vector<Point2d>& sample_points,
			int num_points = 1000, Point2d& sigma = Point2d(0.3, 0.3));
		// ʹ����������㼯����ģ��ƥ��
		float MatchTemplate(const Mat& src, const Mat& templ, Rect2i& match_location,
			MatchMethod match_method, const vector<Point2d>& sample_points);
	public:
		// Ŀ��ģ��
		Mat TargetTemplate;
		// ��ǰ֡���ҵ���Ŀ���
		Rect currentBoundingBox;
		// ��ǰ֡���ҵ���Ŀ��ͼ���
		Mat CurrentTargetPatch;
		// ��һ֡��������Χ
		Rect NextSearchArea;
		// ��Ƶ֡�ľ�������
		Rect FrameArea;
		// ��׼�ض���̫�ֲ������㼯
		vector<Point2d> SamplePoints;
		// �������Ĳ����ṹ��
		Params params;

	};

	typedef SingleTemplateTracker STTracker;
}

