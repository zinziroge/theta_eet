#ifndef __EQUI_LV_H__
#define __EQUI_LV_H__

void dfe_to_equi(
	const IplImage* c2_img,
	IplImage* equi_img,
	const THETA_USBCAM_CIRCLE_T* cam1,
	const THETA_USBCAM_CIRCLE_T* cam2,
	const CvMat* angle_np
	);

void equi_add_grid(IplImage* equi_img);

void equi_rot(
	const IplImage* in_equi_img,
	IplImage* out_equi_img,
	const CvMat* angle_np
	);
void equi_rot(
	const cv::Mat& in_equi_img,
	cv::Mat& out_equi_img,
	const cv::Mat& angle_np
	);

#endif
