// theta_eet.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

// opencv 2.4.1
#include <opencv_lib.h>
#include <opencv2/opencv.hpp>

#include "theta_util.h"
#include "equi_lv.h"
#include "theta_eet.h"

///////////////////////////////////////////////////////////////////////////////
#define		KEY_ESC	'\x1b'

///////////////////////////////////////////////////////////////////////////////


void calc_rot_vec(THETA_EET_CFG_T* cfg, const int width, const int height)
{
	float lng, lat;

	lng = (cfg->np_x / width * 360 - 180)/180*M_PI;
	lat = (-90 + cfg->np_y / height * 180)/180*M_PI;

	cfg->rot_vec[0] = cos(lng)*cos(lat);
	cfg->rot_vec[1] = sin(lat);
	cfg->rot_vec[2] = sin(lng)*(-cos(lat));
}

int isExpo2(const int n)
{
	for(int i=0; i < 32; i++)
		if (n == (1 << i))
			return 1;

	return 0;
}

void write_composite_image(const cv::Mat& composite_img, const int n_frame)
{
	cv::Mat temp_out_img = cv::Mat(composite_img.rows, composite_img.cols, CV_32FC3);

	printf(" write composite image\n");
	temp_out_img = composite_img * 1 / (double)(n_frame + 1);
	char filename[256];
	sprintf(filename, "composite_%04d.tiff", n_frame + 1);
	temp_out_img.convertTo(temp_out_img, CV_8UC3);
	cv::imwrite(filename, temp_out_img);

	temp_out_img.release();
}

void write_rotated_equi_image(const IplImage* equi_rot_img, const int n_frame) {
	printf(" write rotated equi image\n");
	char filename[256];
	sprintf(filename, "equi_rot_%04d.tiff", n_frame + 1);
	//cv::imwrite(filename, equi_rot_img);
	cvSaveImage(filename, equi_rot_img);
}

void write_rotated_equi_image(const cv::Mat& equi_rot_img, const int n_frame) {
	printf(" write rotated equi image\n");
	char filename[256];
	sprintf(filename, "equi_rot_%04d.tiff", n_frame + 1);
	cv::imwrite(filename, equi_rot_img);
}

void theta_eet_mat(FILE* fp_config, THETA_EET_CFG_T* cfg)
{
	cv::Mat frame;
	//IplImage *frame = 0;
	cv::Mat equi_rot_img;
	//IplImage *equi_rot_img = 0;
	cv::Mat frame_preview = cv::Mat::zeros(320, 640, CV_8UC3);
	cv::Mat composite_img;
	int c;
	int n_frame = 0;
	char buf[1024];
	char equi_img_fn[1024];
	int width, height;

	// read 1st image to get size of image.
	fgets(buf, sizeof(buf), fp_config);
	sscanf(buf, "%s", equi_img_fn);
	frame = cv::imread(equi_img_fn);
	height = frame.rows;
	width = frame.cols;
	//frame = cvLoadImage(equi_img_fn);
	//height = frame->height;
	//width = frame->width;
	composite_img = cv::Mat::zeros(height, width, CV_32FC3);
	equi_rot_img = cv::Mat::zeros(height, width, CV_8UC3);
	//equi_rot_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	//
	calc_rot_vec(cfg, width, height);
	printf("x=%10.9f, y=%10.9f, z=%10.9f\n",
		cfg->rot_vec[0], cfg->rot_vec[1], cfg->rot_vec[2]);

	//cv::namedWindow("preview", CV_WINDOW_AUTOSIZE);

	// main
	while (1) {
		// read equi image
		sscanf(buf, "%s", equi_img_fn);
		//cvReleaseImage(&frame);
		//frame = cvLoadImage(equi_img_fn);
		frame.release();
		frame = cv::imread(equi_img_fn);
		if (frame.empty()) {
			fprintf(stderr, "Error: Can't open '%s'.\n", equi_img_fn);
			break;
		}

		// view info
		printf("frame=%04d,image=%s\n", n_frame, equi_img_fn);
		//cv::resize(frame, frame_preview, cv::Size(frame_preview.size()));
		//cv::imshow("preview", frame_preview);
		//c = cvWaitKey(0);

		// rotate equi_image
		//CvMat* rot_vec = cvCreateMat(3, 1, CV_32F);
		cv::Mat rot_vec = cv::Mat(3, 1, CV_32F);

		//cvmSet(rot_vec, 0, 0, cfg->rot_vec[0] * n_frame * cfg->interval_time * ROT_PER_SEC);
		//cvmSet(rot_vec, 1, 0, cfg->rot_vec[1] * n_frame * cfg->interval_time * ROT_PER_SEC);
		//cvmSet(rot_vec, 2, 0, cfg->rot_vec[2] * n_frame * cfg->interval_time * ROT_PER_SEC);
		rot_vec.at<float>(0, 0) = cfg->rot_vec[0] * n_frame * cfg->interval_time * ROT_PER_SEC;
		rot_vec.at<float>(1, 0) = cfg->rot_vec[1] * n_frame * cfg->interval_time * ROT_PER_SEC;
		rot_vec.at<float>(2, 0) = cfg->rot_vec[2] * n_frame * cfg->interval_time * ROT_PER_SEC;

		//equi_rot(&((IplImage)frame), &((IplImage)equi_rot_img), rot_vec);
		equi_rot(frame, equi_rot_img, rot_vec);

		//cv::Mat equi_rot_img_mat = cv::cvarrToMat(equi_rot_img);
		cv::Mat equi_rot_img_32F = cv::Mat(height, width, CV_32FC3);
		//equi_rot_img_mat.convertTo(equi_rot_img_32F, CV_32FC3);
		equi_rot_img.convertTo(equi_rot_img_32F, CV_32FC3);
		cv::add(equi_rot_img_32F, composite_img, composite_img);
		equi_rot_img_32F.release();
		//equi_rot_img_mat.release();
		//cvReleaseMat(&rot_vec);
		rot_vec.release();

		// write composite image when n_fram+1 = 1, 2, 4, 8, 16, ...
		if (isExpo2(n_frame + 1)) {
			write_composite_image(composite_img, n_frame);
		}

		// write rotated equi_img
		write_rotated_equi_image(equi_rot_img, n_frame);

		// ready for next image
		n_frame++;

		if (!fgets(buf, sizeof(buf), fp_config))
			break;

		c = cvWaitKey(1);
		if (c == KEY_ESC)
			break;

	}

	write_composite_image(composite_img, n_frame-1);

	frame.release();
	frame_preview.release();
	composite_img.release();
	equi_rot_img.release();

	//cvDestroyWindow("preview");
}

void theta_eet(FILE* fp_config, THETA_EET_CFG_T* cfg)
{
	//cv::Mat frame;
	IplImage *frame = 0;
	//cv::Mat equi_rot_img;
	IplImage *equi_rot_img = 0;
	cv::Mat frame_preview = cv::Mat::zeros(320, 640, CV_8UC3);
	cv::Mat composite_img;
	int c;
	int n_frame = 0;
	char buf[1024];
	char equi_img_fn[1024];
	int width, height;

	// read 1st image to get size of image.
	fgets(buf, sizeof(buf), fp_config);
	sscanf(buf, "%s", equi_img_fn);
	//frame = cv::imread(equi_img_fn);
	//height = frame.rows;
	//width = frame.cols;
	frame = cvLoadImage(equi_img_fn);
	height = frame->height;
	width = frame->width;
	composite_img = cv::Mat::zeros(height, width, CV_32FC3);
	//equi_rot_img = cv::Mat::zeros(height, width, CV_8UC3);
	equi_rot_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	//
	calc_rot_vec(cfg, width, height);
	printf("x=%10.9f, y=%10.9f, z=%10.9f\n",
		cfg->rot_vec[0], cfg->rot_vec[1], cfg->rot_vec[2]);

	//cv::namedWindow("preview", CV_WINDOW_AUTOSIZE);

	// main
	while (1) {
		// read equi image
		sscanf(buf, "%s", equi_img_fn);
		cvReleaseImage(&frame);
		frame = cvLoadImage(equi_img_fn);
		//frame = cv::imread(equi_img_fn);
		//if (frame.empty()) {
		//	fprintf(stderr, "Error: Can't open '%s'.\n", equi_img_fn);
		//	break;
		//}

		// view info
		printf("frame=%04d,image=%s\n", n_frame, equi_img_fn);
		//cv::resize(frame, frame_preview, cv::Size(frame_preview.size()));
		//cv::imshow("preview", frame_preview);
		//c = cvWaitKey(0);

		// rotate equi_image
		CvMat* rot_vec = cvCreateMat(3, 1, CV_32F);

		cvmSet(rot_vec, 0, 0, cfg->rot_vec[0] * n_frame * cfg->interval_time * ROT_PER_SEC);
		cvmSet(rot_vec, 1, 0, cfg->rot_vec[1] * n_frame * cfg->interval_time * ROT_PER_SEC);
		cvmSet(rot_vec, 2, 0, cfg->rot_vec[2] * n_frame * cfg->interval_time * ROT_PER_SEC);
			
		//equi_rot(&((IplImage)frame), &((IplImage)equi_rot_img), rot);
		equi_rot(frame, equi_rot_img, rot_vec);

		cv::Mat equi_rot_img_mat = cv::cvarrToMat(equi_rot_img);
		cv::Mat equi_rot_img_32F = cv::Mat(height, width, CV_32FC3);
		equi_rot_img_mat.convertTo(equi_rot_img_32F, CV_32FC3);
		cv::add(equi_rot_img_32F, composite_img, composite_img);
		equi_rot_img_32F.release();
		equi_rot_img_mat.release();
		cvReleaseMat(&rot_vec);

		// write composite image when n_fram+1 = 1, 2, 4, 8, 16, ...
		if ( isExpo2(n_frame+1) ) {
			write_composite_image(composite_img, n_frame);
		}

		// write rotated equi_img
		write_rotated_equi_image(equi_rot_img, n_frame);

		// ready for next image
		n_frame++;

		if ( !fgets(buf, sizeof(buf), fp_config) )
			break;

		c = cvWaitKey(1);
		if (c == KEY_ESC )
			break;

	}

	//frame.release();
	frame_preview.release();
	composite_img.release();
	//equi_rot_img.release();
	
	//cvDestroyWindow("preview");
}

void read_config(FILE* fp_config, THETA_EET_CFG_T* cfg) {
	char buf[1024];

	fgets(buf, sizeof(buf), fp_config);
	sscanf(buf, "%f", &(cfg->interval_time));

	fgets(buf, sizeof(buf), fp_config);
	sscanf(buf, "%f %f", &(cfg->np_x), &(cfg->np_y));
}

void usage(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "%s <config.txt>\n", argv[0]);
		exit(1);
	}
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{

	FILE* fp_config;
	THETA_EET_CFG_T cfg;

	usage(argc, argv);

	fp_config = fopen(argv[1], "r");
	if( !fp_config ){
		fprintf(stderr, "Error: Can't open %s.\n", argv[1]);
		exit(1);
	}

	read_config(fp_config, &cfg);

	//theta_eet(fp_config, &cfg);
	theta_eet_mat(fp_config, &cfg);

	fclose(fp_config);

	return 0;
}
