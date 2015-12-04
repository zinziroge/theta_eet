//
// 
//
#include "theta_util.h"

static void equi_xy_to_lnglat(
	const int x,
	const int y,
	const int width,
	const int height,
	double* lng,
	double* lat )
{
	// (x,y)=(0,0) is (lng,lat) = (-M_PI, -M_PI/2)
	*lat = -M_PI / 2 + y / (double)(height)* M_PI;
	*lng = -M_PI + 2 * M_PI * x / (double)(width);
}


void equi_rot(
	const IplImage* in_equi_img,
	IplImage* out_equi_img,
	const CvMat* angle_np
	)
{
	CvMat *rot = cvCreateMat(3, 3, CV_32F);

	cvRodrigues2(angle_np, rot);

	#pragma omp parallel for
	for (int y = 0; y < out_equi_img->height; y++) {
		for (int x = 0; x < out_equi_img->width; x++) {
			double lng, lat;
			double u, v;
			CvScalar col_bgr;
			CvMat *sph_1 = cvCreateMat(3, 1, CV_32F);	// X, Y, Z
			CvMat *sph_2 = cvCreateMat(3, 1, CV_32F);

			// xy(equi)->lnglat
			equi_xy_to_lnglat(x, y, out_equi_img->width, out_equi_img->height, &lng, &lat);

			// lnglat->sph_1(‰º‚ªYŽ²)
			lnglat_to_sph(lng, lat, sph_1);

			// sph_1 -> sph_2
			cvMatMul(rot, sph_1, sph_2);

			// sph_2 -> lnglat
			lng = atan2(-cvmGet(sph_2, SPH_Z, 0), cvmGet(sph_2, SPH_X, 0));
			lat = asin(cvmGet(sph_2, SPH_Y,0 ));
			lng = lng / M_PI * 180;
			lat = lat / M_PI * 180;
			u = (lng + 180) / 360.0 * in_equi_img->width;
			v = (lat + 90) / 180.0 * in_equi_img->height;

			// pixel
			get_pixel(in_equi_img, u, v, &col_bgr);
			set_pixel(out_equi_img, x, y, &col_bgr);

			cvReleaseMat(&sph_1);
			cvReleaseMat(&sph_2);
		}
	}

	cvReleaseMat(&rot);
}