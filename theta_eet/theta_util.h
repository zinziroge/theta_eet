#ifndef __THETA_UTIL_H__
#define __THETA_UTIL_H__

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <opencv_lib.h>
#include <opencv2/opencv.hpp>

///////////////////////////////////////////////////////////////////////////////
typedef struct {
	double cpx;
	double cpy;
	double r;
} THETA_USBCAM_CIRCLE_T;

//typedef struct {
//	double alpha_blending_r_ratio;
//};

//enum SPH_XYZ {
//	SPH_X, SPH,Y, SPH_Z
//};
#define	SPH_X	(0)
#define	SPH_Y	(1)
#define	SPH_Z	(2)

#define	COL_B	(0)
#define COL_G	(1)
#define COL_R	(2)

#define PIX_INTER_NONE			(0)
#define	PIX_INTER_BILINER		(1)

#define LV_CAPTURE			(1)
#define LV_EQUI				(2)
#define LV_LP				(4)
#define LV_CIRCLE			(8)
#define LV_RECTI_1WIN		(16)
#define LV_RECTI_2WIN		(32)
#define LV_RECTI_3WIN		(64)
#define LV_RECTI_4WIN		(128)
#define LV_LP_2				(256)
#define LV_CIRCLE_2			(512)
#define LV_FACE_DET_6WIN	(1024)
#define LV_FACE_6DET_4WIN	(2048)

#define DFE_ALPHA_BLENDING_R_RATIO		(0.95)

///////////////////////////////////////////////////////////////////////////////
int max(int a, int b);
int min(int a, int b);

void get_pixel(
	const IplImage* in_img, 
	const double u, 
	const double v, 
	CvScalar* col_bgr, 
	const int pix_inter_mode = PIX_INTER_BILINER);
void get_pixel(
	const cv::Mat& in_img,
	const double ud,
	const double vd,
	cv::Scalar& col_bgr,
	const int pix_inter_mode = PIX_INTER_BILINER);
void get_dfe_pixel(
	const IplImage* dfe_img,
	const double ud,
	const double vd,
	CvScalar* col_bgr,
	const THETA_USBCAM_CIRCLE_T* cam1,
	const THETA_USBCAM_CIRCLE_T* cam2,
	const int which_cam,
	const double dfe_r);

void set_pixel(IplImage* out_img, const int x, const int y, const CvScalar* col_bgr);
void set_pixel(cv::Mat& out_img, const int x, const int y, const cv::Scalar& col_bgr);

void create_rot_mat(CvMat* rot_x, CvMat* rot_y, CvMat* rot_z, double ang_x, double ang_y, double ang_z);

double rad2deg(const double rad);
double deg2rad(const double deg);

void lnglat_to_equi_uv(
	const double lng,
	const double lat,
	const int width,
	const int height,
	double* u,
	double* v);

void sph_to_lnglat(
	const CvMat* sph_2,
	double* lng,
	double* lat);

void sph_to_lnglat_2(
	const CvMat* sph_2,
	double* lng,
	double* lat);

void lnglat_to_sph(
	const double lng,
	const double lat,
	CvMat* sph_1);
void lnglat_to_sph(
	const double lng,
	const double lat,
	cv::Mat& sph_1);

void sph_to_dfe_uv(
	const CvMat* sph,
	const THETA_USBCAM_CIRCLE_T* cam1,
	const THETA_USBCAM_CIRCLE_T* cam2, 
	double* u,
	double* v,
	int* which_cam,
	double* r
	);

// ToDo
void sph_to_dfe_uv_2(
	const CvMat* sph,
	const THETA_USBCAM_CIRCLE_T* cam1,
	const THETA_USBCAM_CIRCLE_T* cam2,
	double* u,
	double* v,
	int* which_cam,
	double* r
	);

#endif