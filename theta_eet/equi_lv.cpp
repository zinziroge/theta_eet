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
	//*lat = M_PI / 2 - y / (double)(height)* M_PI;
	*lng = -M_PI + 2 * M_PI * x / (double)(width);
	//*lng = 2 * M_PI * x / (double)(width);
}


void equi_rot(
	const IplImage* in_equi_img,
	IplImage* out_equi_img,
	const CvMat* angle_np
	)
{
	//CvMat *rot_x = cvCreateMat(3, 3, CV_32F);
	//CvMat *rot_y = cvCreateMat(3, 3, CV_32F);
	//CvMat *rot_z = cvCreateMat(3, 3, CV_32F);
	CvMat *rot = cvCreateMat(3, 3, CV_32F);
	//CvMat *rot_tmp = cvCreateMat(3, 3, CV_32F);

	// 座標系変換
	//create_rot_mat(rot_x, rot_y, rot_z,
	//	cvmGet(angle_np, SPH_X, 0), cvmGet(angle_np, SPH_Y, 0), cvmGet(angle_np, SPH_Z, 0));
	//cvMatMul(rot_y, rot_x, rot_tmp);
	//cvMatMul(rot_z, rot_tmp, rot);
	cvRodrigues2(angle_np, rot);
	//for (int i = 0; i < 3; i++)
	//	//std::cout << cvmGet(angle_np, i, 0) << "," << std::endl;
	//	printf("%21.20f,\n", cvmGet(angle_np, i, 0));
	//for (int i = 0; i < 9; i++)
	//	//std::cout << cvmGet(rot,i/3,i%3) << "," << std::endl;
	//	printf("%21.20f,\n", cvmGet(rot, i / 3, i % 3));
	//std::cout << std::endl;
	//#pragma omp parallel for  private()
	#pragma omp parallel for
	for (int y = 0; y < out_equi_img->height; y++) {
		//#pragma omp parallel for
		for (int x = 0; x < out_equi_img->width; x++) {
			double lng, lat;	// 経度(0<=lng<=2*M_PI)、緯度(-M_PI/2<=lat<=+M_PI/2)
			double u, v;
			CvScalar col_bgr;
			CvMat *sph_1 = cvCreateMat(3, 1, CV_32F);	// X, Y, Z
			CvMat *sph_2 = cvCreateMat(3, 1, CV_32F);

			// xy(equi)->lnglat
			equi_xy_to_lnglat(x, y, out_equi_img->width, out_equi_img->height, &lng, &lat);

			// lnglat->sph_1(下がY軸)
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

#ifdef __DEBUG
			printf("%4d, %4d, %6.2lf, %6.2lf, %6.2lf, %6.2lf, %6.2lf,,",
				x, y, u, v, rr, theta / M_PI * 180, phi / M_PI * 180);
			printf("s1,%6.2lf, %6.2lf, %6.2lf,,",
				cvmGet(sph_1, SPH_X, 0), cvmGet(sph_1, SPH_Y, 0), cvmGet(sph_1, SPH_Z, 0));
			printf("s2,%6.2lf, %6.2lf, %6.2lf,",
				cvmGet(sph_2, SPH_X, 0), cvmGet(sph_2, SPH_Y, 0), cvmGet(sph_2, SPH_Z, 0));
			printf("\n");
#endif

			cvReleaseMat(&sph_1);
			cvReleaseMat(&sph_2);
		}
	}
	//#pragma omp barrier


	//cvReleaseMat(&rot_x);
	//cvReleaseMat(&rot_y);
	//cvReleaseMat(&rot_z);
	cvReleaseMat(&rot);
	//cvReleaseMat(&rot_tmp);
}