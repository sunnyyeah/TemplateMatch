#include "MultipleTemplateTracker.h"

namespace mycv {
	MultipleTemplateTracker::MultipleTemplateTracker(Params _params)
	{
		cout << "���У�MultipleTemplateTracker::MultipleTemplateTracker()" << endl;
		this->params = _params;
	}


	MultipleTemplateTracker::~MultipleTemplateTracker()
	{
	}

	// ��ʼ��������
	bool MultipleTemplateTracker::init(const Mat& initFrame, const Rect& initBoundingBox) {
		cout << "���У�MultipleTemplateTracker::init()" << endl;
		this->FrameArea = Rect(0, 0, initFrame.cols, initFrame.rows);
		
		
		// ��ȡ��ʼ֡�ϵĳ�ʼ��߶�Ŀ��ģ��
		this->GenerateMultiScaleTargetTemplates(initFrame(initBoundingBox),
			this->MultiScaleTargetsTemplates);
		this->ShowMultiScaleTemplates(this->MultiScaleTargetsTemplates);

		// ������һ֡��������Χ
		this->EstimateSearchArea(initBoundingBox, this->NextSearchArea,
			this->params.expandWidth, this->params.expandWidth);

		// ��ʼ����׼������������㼯
		this->GenerateRandomSamplePoints(this->SamplePoints, this->params.numPoints, this->params.sigma);
		return false;
	}

	// ����Ŀ��
	bool MultipleTemplateTracker::track(const Mat& currentFrame, Rect& currentBoundingBox) {
		cout << "���У�MultipleTemplateTracker::track()" << endl;

		// ��ROI������������Ŀ��
		Rect2i match_location(-1, -1, 0, 0);

		// ���ö�߶�ģ���ڵ�ǰ֡ƥ��
		this->MatchMultiScaleTemplates(currentFrame(this->NextSearchArea), this->MultiScaleTargetsTemplates,
			match_location, this->params.matchMethod, this->params.matchStrategy, this->SamplePoints,
			this->params.xyStep, this->params.xyStride);
		
		// ���µ���ƥ������꣬ʹ�䷵��ԭʼͼ�������ϵ
		match_location.x += this->NextSearchArea.x;
		match_location.y += this->NextSearchArea.y;

		// ���㵱ǰ֡�ϵ�Ŀ��λ��
		this->currentBoundingBox = match_location;

		// ץȡ��ǰ֡��Ŀ��ͼ���
		this->CurrentTargetPatch = currentFrame(this->currentBoundingBox).clone();

		// ������ٽ��
		currentBoundingBox = this->currentBoundingBox;
		return false;
	}

	// ����Ŀ��ģ��
	bool MultipleTemplateTracker::update(Rect& NextSearchBox, double& fps) {
		cout << "���У�MultipleTemplateTracker::update()" << endl;

		t = (double)getTickCount();		//���شӲ���ϵͳ��������ǰ�����ļ�ʱ������

		// ���¶�߶�Ŀ��ģ���
		this->UpdateMultiScaleTargetTemplates(this->CurrentTargetPatch);
		// ��ʾ��߶�Ŀ��ģ��
		this->ShowMultiScaleTemplates(this->MultiScaleTargetsTemplates);


		// ������һ֡�ϵľֲ�������Χ
		this->EstimateSearchArea(this->currentBoundingBox, this->NextSearchArea,
			this->params.expandWidth, this->params.expandWidth);

		// ����ֲ�������Χ
		NextSearchBox = this->NextSearchArea;

		// ����֡��
		t = ((double)getTickCount() - t) / getTickFrequency();
		fps = 1.0 / t;

		return false;
	}

	// ģ��ƥ�亯��
	///����˵��
	/// �ĸ����������ǣ�ԭʼ������ģ�庯�������ص�ƥ���λ�ã�ƥ�䷽��
	float MultipleTemplateTracker::MatchTemplate(const Mat& src, const Mat& templ, Rect2i& match_location,
		MatchMethod match_method, Vec2i& xy_step, Vec2i& xy_stride) {
		assert((src.type() == CV_8UC1) && (templ.type() == CV_8UC1));
		// ԭͼ����ģ��ĳߴ�
		int src_width = src.cols;
		int src_height = src.rows;
		int templ_cols = templ.cols;
		int templ_rows = templ.rows;
		int y_end = src_height - templ_rows + 1;
		int x_end = src_width - templ_cols + 1;

		cout << "x_end: " << x_end << endl;
		cout << "y_end: " << y_end << endl;

		// ��ƥ������У���¼��ƥ���λ�ú�ƥ���
		float match_degree = FLT_MAX;
		int y_match = -1, x_match = -1;

		// ���ϵ���ɨ��ԭͼ��
		for (int y = 0; y < y_end; y += xy_stride[1]) {
			// ������ɨ��ԭͼ��
			for (int x = 0; x < x_end; x += xy_stride[0]) {
				// src(y,x)λ������ģ���ƥ���
				float match_yx = 0.0f;

				// ��ģ�����Ͻ�templ(0,0)���䵽src(y,x)λ�ã���ģ�����ۼ�ÿ���������ص��ϵĲ���
				for (int r = 0; r < templ_rows; r += xy_step[1]) {
					for (int c = 0; c < templ_cols; c += xy_step[0]) {
						uchar src_val = src.ptr<uchar>(y + r)[x + c];
						uchar templ_val = templ.ptr<uchar>(r)[c];
						//cout << "src_val:" << src_val << endl;
						//cout << "templ_val:" << templ_val << endl;
						if (match_method == MatchMethod::SQDIFF)			//SQDIFF
							match_yx += (float)(std::abs(src_val - templ_val) * std::abs(src_val - templ_val));
						if (match_method == MatchMethod::SADIFF)			//SADIFF
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

		match_location = Rect2i(x_match, y_match, templ_cols, templ_rows);
		return match_degree;
	}

	// ������һ֡��������Χ
	void MultipleTemplateTracker::EstimateSearchArea(const Rect& target_location, Rect& search_area, int expand_x, int expand_y) {
		float center_x = target_location.x + 0.5f*target_location.width;
		float center_y = target_location.y + 0.5f*target_location.height;
		search_area.width = target_location.width + expand_x;
		search_area.height = target_location.height + expand_y;
		search_area.x = int(center_x - 0.5f*search_area.width);
		search_area.y = int(center_y - 0.5f*search_area.height);
		search_area &= this->FrameArea;
	}

	// ��ָ���Ĳ�����Χ�ڲ����ض���̫�ֲ��㼯
	void MultipleTemplateTracker::GenerateRandomSamplePoints(vector<Point2d>& sample_points,
		int num_points /*= 1000*/, Point2d& sigma /*= Point2d(0.3, 0.3)*/) {
		RNG rng = theRNG();
		Rect2d sample_area(0.0, 0.0, 1.0, 1.0);
		for (int k = 0; k < num_points;) {
			Point2d pt;
			pt.x = sample_area.width / 2.0 + rng.gaussian(sigma.x);
			pt.y = sample_area.height / 2.0 + rng.gaussian(sigma.y);
			if (sample_area.contains(pt)) {
				sample_points.push_back(pt);
				k++;
			}
		}
	}

	// ʹ����������㼯����ģ��ƥ��
	float MultipleTemplateTracker::MatchTemplate(const Mat& src, const Mat& templ, Rect2i& match_location,
		MatchMethod match_method, const vector<Point2d>& sample_points) {
		CV_Assert((src.type() == CV_8UC1) && (templ.type() == CV_8UC1));

		// ԭͼ���ģ��ĳߴ�
		int src_width = src.cols;
		int src_height = src.rows;
		int templ_cols = templ.cols;
		int templ_rows = templ.rows;
		int y_end = src_height - templ_rows + 1;
		int x_end = src_width - templ_cols + 1;

		/*cout << "x_end: " << x_end << endl;
		cout << "y_end: " << y_end << endl;*/

		// ��Ծ����ģ���С����������������������
		vector<Point2i> SamplePoints(sample_points.size());
		for (size_t k = 0; k < sample_points.size(); k++) {
			const Point2d& ptd = sample_points[k];
			Point2i& pti = SamplePoints[k];				// ���������Ժ�ĵ㼯��Ϊint����
			pti.x = cvFloor(ptd.x * templ_cols);
			pti.y = cvFloor(ptd.y * templ_rows);
		}

		// ��ƥ������У���¼��ƥ���λ�ú�ƥ���
		float match_dgree = FLT_MAX;
		int y_match = -1, x_match = -1;

		// ���ϵ���ɨ��ԭͼ
		for (int y = 0; y < y_end; y++) {
			// ������ɨ��ԭͼ
			for (int x = 0; x < x_end; x++) {
				// src(y,x)λ������ģ���ƥ���
				float match_yx = 0.0f;
				// ���ղ������������ģ����ԭʼͼ���ƥ���
				for (size_t k = 0; k < SamplePoints.size(); k++) {
					Point2i& pt = SamplePoints[k];
					//cout << "pt.x,pt.y:" << pt.x << "," << pt.y << endl;
					uchar src_val = src.ptr<uchar>(y + pt.y)[x + pt.x];
					uchar templ_val = templ.ptr<uchar>(pt.y)[pt.x];
					//cout << "��" << k << "�����src_val��templ_Val-----" << src_val << "    ----" << templ_val << endl;
					if (match_method == MatchMethod::SQDIFF)
						match_yx += float(abs(src_val - templ_val)*abs(src_val - templ_val));
					if (match_method == MatchMethod::SADIFF)
						match_yx += float(abs(src_val - templ_val));
				}
				// ����ʷ��õĲ���Ƚ��бȽϣ��ҳ�������С�ĵ�
				if (match_dgree > match_yx) {
					match_dgree = match_yx;
					x_match = x;
					y_match = y;
				}
			}
		}
		match_location = Rect2i(x_match, y_match, templ_cols, templ_rows);
		return match_dgree;
	}

	// ������߶�Ŀ��ģ��
	void MultipleTemplateTracker::GenerateMultiScaleTargetTemplates(const Mat& origin_target, vector<Mat>& multiscale_target) {
		vector<double> resize_scales = { 1.5, 1.4, 1.3, 1.2, 1.1, 1.0,
			0.9, 0.8, 0.7, 0.6, 0.5 };
		multiscale_target.resize(resize_scales.size(), Mat());
		for (size_t scidx = 0; scidx < resize_scales.size(); scidx++) {
			cv::resize(origin_target, multiscale_target[scidx], Size(), resize_scales[scidx],
				resize_scales[scidx], InterpolationFlags::INTER_AREA);
		}
		return;
	}

	// ��ʾ��߶�ģ��ƥ��
	void MultipleTemplateTracker::ShowMultiScaleTemplates(const vector<Mat>& multiscale_targets) {
		int total_cols = 0, total_rows = 0;
		vector<Rect2i> target_rois(multiscale_targets.size());

		for (size_t k = 0; k < multiscale_targets.size(); k++) {
			target_rois[k] = Rect2i(total_cols, 0, multiscale_targets[k].cols, multiscale_targets[k].rows);
			total_cols += multiscale_targets[k].cols;
			total_rows = max(multiscale_targets[k].rows, total_rows);
		}
		Mat targetsImg = Mat::zeros(total_rows, total_cols, CV_8UC1);
		for (size_t k = 0; k < multiscale_targets.size(); k++) {
			multiscale_targets[k].copyTo(targetsImg(target_rois[k]));
		}
		imshow("Target Image", targetsImg);			// ��ʾroiͼ���
		waitKey(100);
	}

	// ʹ�ö�߶�ģ��ƥ��
	float MultipleTemplateTracker::MatchMultiScaleTemplates(
		const Mat& src, const vector<Mat>& multiscale_templs, Rect2i& best_match_location,
		MatchMethod match_method, MatchStrategy match_strategy,
		const vector<Point2d>& sample_points, Vec2i& xy_step, Vec2i& xy_stride) {
		
		if (match_strategy == MatchStrategy::NORMAL) {
			CV_Assert(!sample_points.empty());
		}

		// ��¼���ƥ��Ⱥ����ƥ��λ��
		float bestMatchDgree = FLT_MAX;
		Rect bestMatchLocation;
		// ��¼ÿ�γ߶�ƥ���λ�ú�ƥ���
		Rect matchLocation;
		float matchDgree;
		// ���Ŷ�߶�ģ����Ŀ��ͼ����ƥ��
		for (size_t scaleIdx = 0; scaleIdx < multiscale_templs.size(); scaleIdx++) {
			const Mat& templ = multiscale_templs[scaleIdx];
			if (match_strategy == MatchStrategy::UNIFORM) {
				matchDgree = this->MatchTemplate(src, templ, matchLocation,
					match_method, xy_step, xy_stride);
			}
			if (match_strategy == MatchStrategy::NORMAL) {
				matchDgree = this->MatchTemplate(src, templ, matchLocation,
					match_method, sample_points);
			}
			// ��¼���ƥ��Ⱥ�ƥ��λ��
			if (matchDgree < bestMatchDgree) {
				bestMatchDgree = matchDgree;
				bestMatchLocation = matchLocation;
			}
		}
		best_match_location = bestMatchLocation;
		return bestMatchDgree;
	}


	// ���¶�߶�Ŀ��ģ��
	void MultipleTemplateTracker::UpdateMultiScaleTargetTemplates(const Mat& currentTargetPatch) {
		for (size_t idx = 0; idx < this->MultiScaleTargetsTemplates.size(); idx++) {
			
			if (this->MultiScaleTargetsTemplates[idx].size() == currentTargetPatch.size()) {
			
				cv::addWeighted(this->MultiScaleTargetsTemplates[idx],
					this->params.alpha, currentTargetPatch, 
					1.0 - this->params.alpha, 0.0, 
					this->MultiScaleTargetsTemplates[idx]);
			}
		}
	}
}