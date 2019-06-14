#include <iostream>
#include <string>
#include <vector>
#include <opencv2\opencv.hpp>

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
			SelectedRoi.x = MIN(x, origin.x);
			SelectedRoi.y = MIN(y, origin.y);
			SelectedRoi.width = std::abs(x - origin.x);
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

// ģ��ƥ�亯��
///����˵��
/// �ĸ����������ǣ�ԭʼ������ģ�庯�������ص�ƥ���λ�ã�ƥ�䷽��
float MatchTemplate(const Mat& src, const Mat& templ, Point2i& match_location, 
	int match_method, Vec2i& xy_step, Vec2i& xy_stride) {
	assert((src.type() == CV_8UC1) && (templ.type() == CV_8UC1));
	// ԭͼ����ģ��ĳߴ�
	int src_width = src.cols;
	int src_height = src.rows;
	int templ_cols = templ.cols;
	int templ_rows = templ.rows;
	int y_end = src_height - templ_rows + 1;
	int x_end = src_width - templ_cols + 1;

	// ��ƥ������У���¼��ƥ���λ�ú�ƥ���
	float match_degree = FLT_MAX;
	int y_match = -1, x_match = -1;

	// ���ϵ���ɨ��ԭͼ��
	for (int y = 0; y < y_end; y+=xy_stride[1]) {
		// ������ɨ��ԭͼ��
		for (int x = 0; x < x_end; x+= xy_stride[0]) {
			// src(y,x)λ������ģ���ƥ���
			float match_yx = 0.0f;

			// ��ģ�����Ͻ�templ(0,0)���䵽src(y,x)λ�ã���ģ�����ۼ�ÿ���������ص��ϵĲ���
			for (int r = 0; r < templ_rows; r+=xy_step[1]) {
				for (int c = 0; c < templ_cols; c+=xy_step[0]) {
					uchar src_val = src.ptr<uchar>(y + r)[x + c];
					uchar templ_val = templ.ptr<uchar>(r)[c];
					//cout << "src_val:" << src_val << endl;
					//cout << "templ_val:" << templ_val << endl;
					if (match_method == 0)			//SQDIFF
						match_yx += (float)(std::abs(src_val - templ_val) * std::abs(src_val - templ_val));
					if (match_method == 1)			//SADIFF
						match_yx += (float)(abs(src_val - templ_val));
					//cout << "match_yx:" << match_yx << endl;
				}
			}
			// ����ʷ��õĲ���Ƚ��бȽϣ��ҳ�������С�ĵ�
			if (match_degree > match_yx) {
				match_degree = match_yx;
				x_match = x;
				y_match = y;
			}
		}
	}

	match_location = Point2i(x_match, y_match);
	return match_degree;
}

int main(int argc, char** argv) {
	const string image_file = "08.jpeg";   

	Mat srcImg = imread(image_file, IMREAD_GRAYSCALE);

	// ������ʾ���
	Mat displayImg;
	srcImg.copyTo(displayImg);

	// ����ȫ�ֱ���
	global::isRoiReady = false;
	global::selectObject = false;
	global::displayImg = displayImg;

	const string winName = "Result Image";
	namedWindow(winName, WINDOW_AUTOSIZE);
	setMouseCallback(winName, global::onMouse, 0);

	// ѭ����ʾͼ�� �ȴ����ѡ��ROI����
	for (;;) {
		imshow(winName, displayImg);

		// һ��ѡ���ROI���򣬾ͽ��봦��
		if (global::isRoiReady) {
			// ����Ϊfalse��������˴ξͽ��ŵȴ����ѡ��
			global::isRoiReady = false;

			// ��ȡ���ѡ�е�ͼ���
			Rect roiRect = global::SelectedRoi;
			Mat roiImg = srcImg(roiRect).clone();
			imshow("ROI Image", roiImg);				//��ʾroiͼ���

			// Ϊԭʼͼ����Ӹ�˹����
			Mat noiseImg(srcImg.size(), srcImg.type());
			randn(noiseImg, Scalar(0), Scalar(30));		//��ֵ��0����׼�30

			Mat workImg = noiseImg + srcImg;			//����ͼ��
			// ��ʾ������Ⱦ��ͼ���ѡ���ROI����
			workImg.copyTo(displayImg);
			rectangle(displayImg, global::SelectedRoi, Scalar::all(0), 4);
			imshow(winName, displayImg);
			waitKey(15);

			// ѡ���ģ���ڱ�������Ⱦ��ͼ����ƥ��
			Point2i match_location;
			int match_method = 1;
			Vec2i xy_step(2, 2);		//ģ����ԭͼ���ص�ƥ��Ĳ���
			Vec2i xy_stride(8, 8);		//ģ����ԭͼ���ƶ��Ĳ��������ڴ��roi������ԣ��������������̫�󣬵��Ƕ���С��roi������ԣ��󲽳����������
			float matchDegree = MatchTemplate(workImg, roiImg, match_location, match_method, xy_step, xy_stride);
			Rect matchedRoi(match_location.x, match_location.y, roiImg.cols, roiImg.rows);
			cout << "ƥ��ȣ�" << matchDegree << endl;
			cout << "ƥ��λ�ã�" << matchedRoi << endl;
			// ��ʾƥ����
			rectangle(displayImg, matchedRoi, Scalar::all(255), 2);
			imshow(winName, displayImg);
			waitKey(15);
		}

		waitKey(15);
	}

	waitKey(0);
	return 0;
}