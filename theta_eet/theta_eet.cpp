// theta_eet.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

/*
http://www.subblue.com/blog/2010/6/17/little_planets
http://mathworld.wolfram.com/StereographicProjection.html
*/

#include "stdafx.h"
#include <opencv_lib.h>
#include <opencv2/opencv.hpp>
#include "theta_util.h"
#include "equi_lv.h"

#define		ROT_PER_SEC	(-360.0f/24.0/3600.0/180.0*M_PI)	// [rad]
#define		KEY_ESC	'\x1b'

///////////////////////////////////////////////////////////////////////////////
CvPoint g_prev_mp = cvPoint(0, 0);
//double g_data_equi_rot[] = { 0, 0, 0 };
#define		NP_ROT	(-360.0f/24.0/3600.0*8/180.0*M_PI)
//float g_data_equi_rot[] = { 
//	-0.647755259, 
//	-0.582477697,
//	-0.491052804 };
float g_data_equi_rot[] = {
	0.620081225,
	-0.548749271,
	-0.56069021 };
//float g_data_equi_rot[] = { M_PI*10/180, 0, 0 };
//float g_data_equi_rot[] = { 0.904508497*M_PI * 45 / 180.0, 0.309016994 * M_PI * 45 / 180, -0.293892626 * M_PI * 45 / 180 };
CvMat g_equi_angle = cvMat(3, 1, CV_32F, g_data_equi_rot);

///////////////////////////////////////////////////////////////////////////////

typedef struct {
	float np_x;
	float np_y;
	float rot_vec[3];	// rotation unit vector
	float interval_time;
} THETA_EET_CFG_T;

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
	composite_img = cv::Mat::zeros(height, width, CV_16UC3);
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
		{
			CvMat* rot = cvCreateMat(3, 1, CV_32F);

			cvmSet(rot, 0, 0, cfg->rot_vec[0] * n_frame * cfg->interval_time * ROT_PER_SEC);
			cvmSet(rot, 1, 0, cfg->rot_vec[1] * n_frame * cfg->interval_time * ROT_PER_SEC);
			cvmSet(rot, 2, 0, cfg->rot_vec[2] * n_frame * cfg->interval_time * ROT_PER_SEC);
			
			//equi_rot(&((IplImage)frame), &((IplImage)equi_rot_img), rot);
			equi_rot(frame, equi_rot_img, rot);

			cv::Mat equi_rot_img_mat = cv::cvarrToMat(equi_rot_img);
			cv::Mat equi_rot_img_16U = cv::Mat(height, width, CV_16UC3);
			equi_rot_img_mat.convertTo(equi_rot_img_16U, CV_16UC3);
			cv::add(equi_rot_img_16U, composite_img, composite_img);
			equi_rot_img_16U.release();

			cvReleaseMat(&rot);
		}

		// write composite image
		if ( isExpo2(n_frame+1) ) {
			cv::Mat temp_out_img = cv::Mat(height, width, CV_16UC3);

			printf(" write composite image\n");
			temp_out_img = composite_img * 1 / (double)(n_frame + 1);
			char filename[256];
			sprintf(filename, "composite_%04d.tiff", n_frame + 1);
			temp_out_img.convertTo(temp_out_img, CV_8UC3);
			cv::imwrite(filename, temp_out_img);

			temp_out_img.release();
		}

		// write rotated equi_img
		{
			printf(" write rotated equi image\n");
			char filename[256];
			sprintf(filename, "equi_rot_%04d.tiff", n_frame + 1);
			//cv::imwrite(filename, equi_rot_img);
			cvSaveImage(filename, equi_rot_img);
		}

		//n_frame += 127;
		n_frame++;

		if ( !fgets(buf, sizeof(buf), fp_config) )
			break;

		c = cvWaitKey(1);
		if (c == KEY_ESC )
			break;

	}

	//cvSaveImage("out.tiff", equi_img);
	//IplImage* cross_img = cvCreateImage(cvSize(640, 320), IPL_DEPTH_8U, 3);
	//for (double y = 0; y < cross_img->height; y += cross_img->height / 10.0)
	//	cvDrawLine(cross_img, cvPoint(0, y), cvPoint(cross_img->width, y), CV_RGB(rand() % 255, rand() % 255, rand() % 255), 1);
	//for (double x = 0; x < cross_img->width; x += cross_img->width / 20.0)
	//	cvDrawLine(cross_img, cvPoint(x, 0), cvPoint(x, cross_img->height), CV_RGB(rand() % 255, rand() % 255, rand() % 255), 1);
	//cvSaveImage("cross.jpg", cross_img);
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

	//
	//int file_num = 12034;
	//int file_num = 12179;

	usage(argc, argv);

	fp_config = fopen(argv[1], "r");
	if( !fp_config ){
		fprintf(stderr, "Error: Can't open %s.\n", argv[1]);
		exit(1);
	}

	read_config(fp_config, &cfg);

	theta_eet(fp_config, &cfg);

	fclose(fp_config);

	return 0;
}
