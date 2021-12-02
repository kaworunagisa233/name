#include<opencv2/opencv.hpp>
#include "opencv2/imgproc/types_c.h" 
#include<iostream>

using namespace std;
using namespace cv;

Mat ellipse_detect(Mat& src)
{
	Mat img = src.clone();
	Mat skinCrCbHist = Mat::zeros(Size(256, 256), CV_8UC1);
	//����opencv�Դ�����Բ���ɺ���������һ����ɫ��Բģ��
	ellipse(skinCrCbHist, Point(113, 155.6), Size(23.4, 15.2), 43.0, 0.0, 360.0, Scalar(255, 255, 255), -1);
	Mat ycrcb_image;
	Mat output_mask = Mat::zeros(img.size(), CV_8UC1);
	cvtColor(img, ycrcb_image, CV_BGR2YCrCb); //����ת���ɵ�YCrCb�ռ�
	for (int i = 0; i < img.cols; i++)   //������ԲƤ��ģ�ͽ���Ƥ�����
		for (int j = 0; j < img.rows; j++)
		{
			Vec3b ycrcb = ycrcb_image.at<Vec3b>(j, i);
			if (skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) > 0)   //���������Ƥ��ģ����Բ�����ڣ��õ����Ƥ�����ص�
				output_mask.at<uchar>(j, i) = 255;
		}

	Mat detect;
	img.copyTo(detect, output_mask);  //���ط�ɫͼ
	return detect;
}

Mat YCrCb_Otsu_detect(Mat& src)
{
	Mat ycrcb_image;
	cvtColor(src, ycrcb_image, CV_BGR2YCrCb); //����ת���ɵ�YCrCb�ռ�
	Mat detect;
	vector<Mat> channels;
	split(ycrcb_image, channels);
	Mat output_mask = channels[1];
	threshold(output_mask, output_mask, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	src.copyTo(detect, output_mask);
	return detect;

}

Mat skinDetect(const cv::Mat& image, double minHue, double maxHue, double minSat, double maxSat)
{
	//����HSVͼ��
	cv::Mat hsv;
	cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
	std::vector<cv::Mat> channels;
	cv::split(hsv, channels);
	//����������Сɫ��������
	cv::Mat mask1;
	cv::threshold(channels[0], mask1, minHue, 255, cv::THRESH_BINARY);
	//����С�����ɫ��������
	cv::Mat mask2;
	cv::threshold(channels[0], mask2, maxHue, 255, cv::THRESH_BINARY_INV);

	//����ɫ������
	cv::Mat hueMask;
	//����������Сɫ��С�����ɫ����ȡ��
	if (minHue < maxHue)
	{
		hueMask = mask1 & mask2;
	}
	//����������Сɫ�Ĵ������ɫ�����������0��������
	else
	{
		hueMask = mask1 | mask2;
	}

	//�������Ͷ�����
	cv::Mat satMask;
	//���ڱ��Ͷ�û�������ߣ�ֱ�ӵ���cv::inRange����������ֵͼ����
	cv::inRange(channels[1], minSat, maxSat, satMask);
	return hueMask & satMask;
}

Mat change_contrast(Mat src1) {
	int height = src1.rows;//���src1�ĸ�
	int width = src1.cols;//���src1�Ŀ�
	Mat dst = Mat::zeros(src1.size(), src1.type());  //������Ҫ������һ����ԭͼһ����С�Ŀհ�ͼƬ              
	float alpha = 1.4;//�����Աȶ�Ϊ1.5
	float beta = 5;//�������ȼ�
	//ѭ������������ÿһ�У�ÿһ�е�Ԫ��
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (src1.channels() == 3)//�ж��Ƿ�Ϊ3ͨ��ͼƬ
			{
				//�������õ���ԭͼ����ֵ�����ظ�����b,g,r
				float b = src1.at<Vec3b>(row, col)[0];//nlue
				float g = src1.at<Vec3b>(row, col)[1];//green
				float r = src1.at<Vec3b>(row, col)[2];//red
				//��ʼ�������أ��Ա���b,g,r���ı���ٷ��ص��µ�ͼƬ��
				dst.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(b * alpha + beta);
				dst.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(g * alpha + beta);
				dst.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(r * alpha + beta);
			}
		}
	}

	return dst;
}

int main(int argc, char** argv) {

	Mat image = imread("./pictures/3.jpg");
	//Mat nana = image;
	Mat mask, img_1, img_ellipse, img_Otsu, img_processed, result;
	//���õ�ɫ����ΧΪ160~15֮��(320~30)�����Ͷȷ�Χ��25~180֮��(0.1~0.65)
	mask = skinDetect(image, 160, 15, 55, 166);
	cv::Mat detected(image.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	//ͨ��copyTo������ķ�����ԭͼ���������ͼ��
	image.copyTo(detected, mask);
	img_1 = change_contrast(image);
	if (image.empty()) {
		printf("could not load image...\n");
		return -1;

	}
	//pyrDown(image, img);
	//namedWindow("test_opencv_setup", 0);
/*	for (int i = 0; i<image.rows; i++) {
		image.at<Vec3b>(i, 10)[0] = 255;
		image.at<Vec3b>(i, 10)[1] = 0;
		image.at<Vec3b>(i, 10)[2] = 0;
	}*/
	img_ellipse = ellipse_detect(image);
	img_Otsu = YCrCb_Otsu_detect(image);
	img_processed = img_ellipse & img_Otsu & detected;

	//medianBlur(img_processed, result, 3);
	imshow("example1", img_ellipse);
	imshow("example2", img_Otsu);
	imshow("detected", detected);


	img_ellipse = ellipse_detect(img_1);
	img_Otsu = YCrCb_Otsu_detect(img_1);
	img_processed = img_ellipse & img_Otsu & detected;
	imshow("example3", img_ellipse);
	imshow("example4", img_Otsu);
	//imshow("detected", detected);

	imshow("contrast_chaged", img_1);
	//imshow("finest", img_processed);
	//imshow("most excellent", result);

	waitKey(0);
	return 0;

}

