#include "Tracker.h"

namespace mycv {
	// ���캯��
	Tracker::Tracker()
	{
		cout << "���У�Tracker::Tracker()" << endl;
	}

	// ��������
	Tracker::~Tracker()
	{
	}

	// ��ʼ��������
	bool Tracker::init(const Mat& initFrame, const Rect& initBoundingBox) {
		cout << "���У�Tracker::init()" << endl;
		return false;
	}

	// ����Ŀ��
	bool Tracker::track(const Mat& currentFrame, Rect& currentBoundingBox) {
		cout << "���У�Tracker::track()" << endl;
		return false;
	}

	// ����Ŀ��ģ��
	bool Tracker::update(Rect& NextSearchBox, double& fps) {
		cout << "���У�Tracker::update()" << endl;
		return false;
	}
}
