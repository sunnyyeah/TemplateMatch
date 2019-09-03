#pragma once   //֮ǰû�������������̳������������

#include <iostream>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

namespace mycv {

	// Ŀ�������
	class Tracker
	{
	public:
		Tracker();
		virtual ~Tracker();
		// ��ʼ��������
		virtual bool init(const Mat& initFrame, const Rect& initBoundingBox);   //Ҫ����Щ���������virtual���Ż��ڵ���ʱ��������ĺ���
		// ����Ŀ��
		virtual bool track(const Mat& currentFrame, Rect& currentBoundingBox);
		// ����Ŀ��ģ��
		virtual bool update(Rect& NextSearchBox, double& fps);
	};

}

