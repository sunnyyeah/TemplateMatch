///<summary>
///��߶�Ŀ�����
///</summary>

#include <iostream>
#include <string>
#include <vector>
#include <opencv2\opencv.hpp>
#include "Tracker.h"
#include "SingleTemplateTracker.h"
#include "MultipleTemplateTracker.h"
#include "datasets.h"

using namespace std;
using namespace cv;

// �������ѡ������
namespace global {
	bool paused = true;			//��������Ҽ�����ͣ��־
	Mat displayImg;				//����ѡ��Ŀ��ʱ�����϶��ۼ�
	bool selectObject = false;	//selectObject�ĳ�ʼֵΪfalse
	bool isRoiReady = false;	//ROI�����Ƿ��Ѿ�ѡ���
	Point origin;				//ROI��������Ͻ���ʼλ��
	Rect SelectedRoi;			//����ͨ�����ѡ���ROI����

	// �����괥���¼�
	static void onMouse(int event, int x, int y, int, void*) {
		if (selectObject) {		//�����������º󣬸ö���俪ʼִ��
								//��ס����϶�����ʱ�򣬸������Ӧ�����ᱻ���ϵĴ��������ϼ���Ŀ����εĴ���
			SelectedRoi.x = MIN(x, origin.x);			// xΪ�������ͼ����x���������λ��
			SelectedRoi.y = MIN(y, origin.y);
			SelectedRoi.width = std::abs(x - origin.x);	// ģ��ͼ��Ŀ��
			SelectedRoi.height = std::abs(y - origin.y);
			SelectedRoi &= Rect(0, 0, displayImg.cols, displayImg.rows);	//����Խ��

			//��������ѡ���
			rectangle(displayImg, SelectedRoi, Scalar(0, 0, 255), 1);
		}

		switch (event)
		{
			//���ڵ�һ֡������������select0bject����Ϊtrue
		case  EVENT_LBUTTONDOWN:
			origin = Point(x, y);
			SelectedRoi = Rect(x, y, 0, 0);
			selectObject = true;
			isRoiReady = false;
			break;
			//ֱ��������̧�𣬱�־��Ŀ������ѡ����ϣ�selectObject����Ϊfalse
		case EVENT_LBUTTONUP:
			selectObject = false;
			if (SelectedRoi.width > 0 && SelectedRoi.height > 0)
				isRoiReady = true;
			cout << "Ŀ�������Ѿ�ѡ�����" << endl;
			cout << "ѡ�еľ�������Ϊ��" << SelectedRoi << endl;
			break;
			// �����Ҽ�����ͣ/��ʼ
		case EVENT_RBUTTONDOWN:
			paused = !paused;
			break;
		}
	}
}

//��Ƶ���ݼ�
//namespace datsets {
//	const string datasets_dir = "D:\\0_Study\\Opencv\\Workspace\\Tracking\\video\\";
//	const string video1 = datasets_dir + "VID_20181121_183353.mp4";
//	int video1_start_frame = 30;
//
//	const string video = video1;
//	int start_frame = video1_start_frame;
//}



int main(int argc, char* argv[]) {
	// ����֡��
	double fps;
	//���ڴ��֡�ʵ��ַ���
	char stringFps[10];		
	//���ڴ��֡�����ַ���
	char stringframe[10];

	// ָ�����ݼ�����ʼ֡����ʵĿ��
	mycv::DataSet dataset = mycv::dataset11;

	VideoCapture capture;

	// ����Ƶ
	capture.open(dataset.video_name);
	assert(capture.isOpened());

	// ��ȡ��Ƶ��Ϣ
	const int FrameCount = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);
	const int FrameWidth = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
	const int FrameHeight = (int)capture.get(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
	const Rect FramArea(0, 0, FrameWidth, FrameHeight);

	// ���ôӵڼ�֡��ʼ��ȡ
	int frameIndex = dataset.start_frame;
	capture.set(VideoCaptureProperties::CAP_PROP_POS_FRAMES, double(frameIndex));

	// ������ʾ���ٹ��̵Ĵ���
	const string winName = "Tracking Window";
	namedWindow(winName, 1);
	// ���ûص�����
	setMouseCallback(winName, global::onMouse, 0);		

	// ��ȡָ������ʼ֡
	Mat CurrentFrame, WorkFrame;
	capture >> CurrentFrame;
	assert(!CurrentFrame.empty());
	cout << "��ǰ֡������" << frameIndex << endl;
	frameIndex++;

	// ����ʼ֡��ѡ��Ŀ������
	while (!global::isRoiReady)
	{
		// ����ʼ֡������displayImg��
		CurrentFrame.copyTo(global::displayImg);

		// �ڰ�����������̧��������֮������ʱ�䣬selectObjectΪtrue��
		// selectedRoi�����������ƶ����ϱ仯��ֱ��̧����������
		//selectObjectΪfalse��selectedRoi����ѡ�е�Ŀ����ο�
		if (global::selectObject && global::SelectedRoi.width > 0 && global::SelectedRoi.height > 0) {
			
			Mat roi_img(global::displayImg, global::SelectedRoi);			
			bitwise_not(roi_img, roi_img);		//��ѡ�е�����ͼ��ת��ʾ		
		}

		// ��ʾ���ѡ�����
		imshow(winName, global::displayImg);
		waitKey(10);
	}

	// ���lock_roi==true���ͱ�ʾ���ѡ���������Ч
	if (dataset.lock_roi)
		global::SelectedRoi = dataset.start_roi;
	cout << "��ʼ֡�ϵ�Ŀ��λ�ã�" << global::SelectedRoi << endl;

	cout << "��������������ʵ������ʼ��Ŀ�������..." << endl;
	//mycv::STTracker::Params params = mycv::STTracker::Params();
	//params.numPoints = 1000;			// �޸�Ĭ�ϲ���
	//Ptr<mycv::Tracker> tracker = new mycv::SingleTemplateTracker(params);
	
	mycv::MTTracker::Params mtparams = mycv::MTTracker::Params();
	mtparams.alpha = 0.7;				// �޸�Ĭ�ϲ���
	mtparams.numPoints = 1000;			
	mtparams.sigma = Point2d(0.4, 0.4);
	mtparams.expandWidth = 50;
	Ptr<mycv::Tracker> tracker = new mycv::MultipleTemplateTracker(mtparams);

	cvtColor(CurrentFrame, WorkFrame, COLOR_BGR2GRAY);
	tracker->init(WorkFrame, global::SelectedRoi);
	
	cout << "��������Ҽ���������...." << endl;

	// ����ѭ����������Ƶ���У�����Ŀ��
	for (; frameIndex < FrameCount;) {
		// ���û����ͣ�������������һ֡ͼ��
		if (!global::paused) {
			capture >> CurrentFrame;
			assert(!CurrentFrame.empty());
			cout << "��ǰ֡������" << frameIndex << endl;
			frameIndex++;

			// �������ͼ�񿽱���displayImg��
			CurrentFrame.copyTo(global::displayImg);
			// ת��Ϊ�Ҷ�ͼ��
			cvtColor(CurrentFrame, WorkFrame, COLOR_BGR2GRAY);

			// ��ʼ����
			Rect currentBoundingBox;
			tracker->track(WorkFrame, currentBoundingBox);

			// ����Ŀ��ģ��
			Rect NextSearchBox;
			tracker->update(NextSearchBox, fps);

			// ��ʾ֡��
			sprintf(stringFps, "%.2f", fps);		//֡�ʱ�����λС��
			std::string fpsString("FPS:");
			fpsString += stringFps;
			putText(global::displayImg, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

			// ��ʾ֡��
			sprintf(stringframe, "%d", frameIndex);		//֡�ʱ�����λС��
			std::string frameString("frameIndex:");
			frameString += stringframe;
			putText(global::displayImg, frameString, Point(5, 35), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

			// ��ʾ��ǰ֡���ٽ��ͼ��
			rectangle(global::displayImg, NextSearchBox, Scalar(255, 0, 0), 2);
			rectangle(global::displayImg, currentBoundingBox, Scalar(0, 0, 255), 2);
			imshow(winName, global::displayImg);

			// ����ÿ֡ͼƬ
			string imageFileName;   
			stringstream StrStm;
			StrStm.clear();
			imageFileName.clear();
			StrStm << "./Image/" << frameIndex-10;
			StrStm >> imageFileName;
			imageFileName += ".jpg";
			imwrite(imageFileName, global::displayImg);

			waitKey(30);
		}
		else
		{
			// ��ʾ��ǰ֡���ٽ��ͼ��
			imshow(winName, global::displayImg);
			waitKey(300);
		}
	}
	return 0;
}