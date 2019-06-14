#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace cv;

int main() {
	ifstream fin("calibdata9.txt"); /* �궨����ͼ���ļ���·�� */
	ofstream fout("caliberation_result9.txt");  /* ����궨������ļ� */

	//��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��   
	cout << "����ʼ��ȡ�ǵ㡿\n";
	int image_count = 0;  /* ͼ������ */
	Size image_size;  /* ͼ��ĳߴ� */
	//Size board_size = Size(7, 6);    /* �궨����ÿ�С��еĽǵ��� */
	Size board_size = Size(13, 8);
	vector<Point2f> image_points_buf;  /* ����ÿ��ͼ���ϼ�⵽�Ľǵ�λ�ã�����λ�õ�ÿ�����ֶ������������걣�棩 */
	vector<vector<Point2f>> image_points_seq; /* �����⵽�����нǵ� */
	string filename;
	int count = 0;//���ڴ洢�ǵ������  

	///
	/// -------------------------------------------��ȡ�ǵ�-------------------------------------------------
	///
	while (getline(fin, filename))
	{
		image_count++;
		// ���ڹ۲�������  
		cout << "image_count = " << image_count << endl;
		//cout << "-->count = " << count<<endl;
		Mat imageInput = imread(filename);   //��ȡ��ǰͼƬ
		if (image_count == 1)  //�����һ��ͼƬʱ��ȡͼ������Ϣ  
		{
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
		}

		//imshow("image", imageInput);
		//waitKey(20);

		/* ��ȡ�ǵ� */
		if (!findChessboardCorners(imageInput, board_size, image_points_buf))
		{
			cout << "can not find chessboard corners!\n"; //�Ҳ����ǵ�  
			//exit(1);
		}
		else
		{
			Mat view_gray;
			cvtColor(imageInput, view_gray, COLOR_RGB2GRAY);   //��ͼ��ת��Ϊ�Ҷ�ͼ

			/* �����ؾ�ȷ�� */
			cornerSubPix(view_gray, image_points_buf, Size(5, 5), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.1));
			//�����ؾ�ȷ��������
			//Size(5,5)���������ڵĴ�С,Size(-1,-1)��ʾû������
			//���ĸ�����������ǵ�ĵ������̵���ֹ����������Ϊ���������ͽǵ㾫�����ߵ����

			count += image_points_buf.size();
			image_points_seq.push_back(image_points_buf);//���������ؽǵ�

			/*�������̽ǵ�*/
			drawChessboardCorners(view_gray, board_size, image_points_buf, false);
			//���ڻ��Ʊ��ɹ��궨�Ľǵ㣬����8λ�ҶȻ��߲�ɫͼ��
			//���ĸ������Ǳ�־λ������ָʾ����������ڽǵ��Ƿ�������̽�⵽
			//false��ʾ��δ��̽�⵽���ڽǵ㣬��ʱ��������ԲȦ��ǳ���⵽���ڽǵ�

			namedWindow("Camera Calibration", WINDOW_AUTOSIZE);
			imshow("Camera Calibration", view_gray);//��ʾͼƬ
			waitKey(500);
		}
	}


	// ������нǵ�
	int total = image_points_seq.size();					 //�ܹ��ж�����ͼƬ�ɹ������˽ǵ�
	cout << "total = " << total << endl << endl;
	int CornerNum = board_size.width*board_size.height;		//ÿ��ͼƬ���ܵĽǵ���  
	for (int ii = 0; ii<total; ii++)
	{
		int k = -1;
		k = ii % total;										 // 13 ��ÿ��ͼƬ���������ж������Ϊ����� ͼƬ�ţ����ڿ���̨�ۿ�
			
		int j = k + 1;
		cout << "����" << j << "ͼƬ�����ݡ� : " << endl;

		//������еĽǵ�  ��ii��ͼƬ�ĵ�i����
		for (int i = 0; i < CornerNum; i++) {
			cout << "(" << image_points_seq[ii][i].x;
			cout << ", " << image_points_seq[ii][i].y << ")";
			if (0 == (i+1)% 6)								 // ���ж���䣬��ʽ����������ڿ���̨�鿴  
			{
				cout << endl;
			}
			else
			{
				cout.width(10);
			}
		}
		cout << endl;
	}
	cout << "�ǵ���ȡ��ɣ�\n" << endl;

	/// 
	/// ----------------------------------------------------������������궨  ------------------------------------------
	/// 
	cout << "����ʼ�궨��" << endl;
	/*������ά��Ϣ*/
	//Size square_size = Size(200, 200);
	Size square_size = Size(150, 150);						 // ʵ�ʲ����õ��ı궨����ÿ�����̸�Ĵ�С 
	vector<vector<Point3f>> object_points;					 // ��������ͼƬ�ı궨���Ͻǵ����ά���꣨��������ʵ���꣩ 
	
	/*�������*/
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ������ڲ������� 
	vector<int> point_counts;								// ÿ��ͼ���нǵ������  
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));	// �������5������ϵ����k1,k2,p1,p2,k3 
	vector<Mat> tvecsMat;									// ÿ��ͼ���ƽ������ 
	vector<Mat> rvecsMat;									// ÿ��ͼ�����ת���� 
						  
	/* ��ʼ���궨���Ͻǵ���ʵ����ά���� */
	int i, j, t;
	for (t = 0; t<image_count; t++)
	{
		vector<Point3f> tempPointSet;
		for (i = 0; i<board_size.height; i++)
		{
			for (j = 0; j<board_size.width; j++)
			{
				Point3f realPoint;
				/* ����궨�������������ϵ��z=0��ƽ���� */
				realPoint.x = i * square_size.width;
				realPoint.y = j * square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}


	/* ��ʼ��ÿ��ͼ���еĽǵ��������ٶ�ÿ��ͼ���ж����Կ��������ı궨�� */
	for (i = 0; i<image_count; i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}

	/* ��ʼ�궨 */
	calibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
	cout << "���궨��ɣ���\n\n";

	//�Ա궨�����������  
	cout << "����ʼ���۱궨�����\n";
	double total_err = 0.0;								// ����ͼ���ƽ�������ܺ� 
	double err = 0.0;									// ÿ��ͼ���ƽ����� 
	vector<Point2f> image_points2;						// �������¼���õ�����ͶӰ�� 
	cout << "\t*ÿ��ͼ��ı궨���*\n";
	fout << "ÿ��ͼ��ı궨��\n";
	for (i = 0; i<image_count; i++)
	{
		vector<Point3f> tempPointSet = object_points[i];
		/* ͨ���õ������������������Կռ����ά���������ͶӰ���㣬�õ��µ�ͶӰ�� */
		projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points2);
		/* �����µ�ͶӰ��;ɵ�ͶӰ��֮������*/
		vector<Point2f> tempImagePoint = image_points_seq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(image_points2[j].x, image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		//������
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err /= point_counts[i];
		cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
	}
	cout << "����ƽ����" << total_err / image_count << "����" << endl;
	fout << "����ƽ����" << total_err / image_count << "����" << endl << endl;
	cout << "��������ɣ���\n" << endl;
	//���涨����      
	cout << "����ʼ���涨������" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0));				// ����ÿ��ͼ�����ת����
	fout << "����ڲ�������" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "����ϵ����\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i<image_count; i++)
	{
		fout << "��" << i + 1 << "��ͼ�����ת������" << endl;
		fout << rvecsMat[i] << endl;
		/* ����ת����ת��Ϊ���Ӧ����ת���� */
		Rodrigues(rvecsMat[i], rotation_matrix);
		fout << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fout << rotation_matrix << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
		fout << tvecsMat[i] << endl << endl;
	}
	cout << "����ɱ��桿\n" << endl;
	fout << endl;

	/************************************************************************
	��ʾ������
	*************************************************************************/
	Mat mapx = Mat(image_size, CV_32FC1);
	Mat mapy = Mat(image_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);
	cout << "���������ͼ��" << endl;
	string imageFileName;   //����ͼ��
	string imageFileName1;   //���ͼ��
	stringstream StrStm;
	stringstream StrStm1;

	for (int i = 1; i <= image_count; i++)
	{
		cout << "Frame # " << i << "....." << endl;
		cout << cameraMatrix << endl;
		cout << distCoeffs << endl;
		initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix,
								image_size, CV_32FC1, mapx, mapy);				//�����������ӳ��

		StrStm.clear();//�������
		StrStm1.clear();
		imageFileName.clear();
		imageFileName1.clear();
		//string filePath = "left";
		string filePath = "./Self11/pic";
		//StrStm << setw(2) << setfill('0') << i;
		StrStm << i;     //������i
		StrStm >> imageFileName;   //����������ݴ���imageFileName
		filePath += imageFileName;
		filePath += ".jpg";
		//��ȡͼƬ·��
		Mat imageSource = imread(filePath);//��ȡͼ��
		Mat newimage = imageSource.clone();//����ͼ��

		
		remap(imageSource, newimage, mapx, mapy, INTER_LINEAR);//����õ�ӳ��Ӧ�õ�ͼ����
															   //��initUndistortRectifyMap���ʹ�ã�Ϊ��������֮һ

															   //undistort(imageSource,newimage,cameraMatrix,distCoeffs);//����������
															   //���������newCameraMatrix=noArray()��Ĭ�ϸ�cameraMatrix����һ��,�ʿ�ʡ

		StrStm1 << "./Result/result" << i;
		StrStm1 >> imageFileName1;
		imageFileName1 += "_d.jpg";								//������ͼƬ����
		imwrite(imageFileName1, newimage);						//����������ͼƬ
		imshow("Original Image", imageSource);
		//waitKey(500);//��ͣ0.5s
		imshow("Undistorted Image", newimage);
		waitKey(500);

	}
	fin.close();
	fout.close();
	getchar();//�ȴ��������˳�
	return 0;
}