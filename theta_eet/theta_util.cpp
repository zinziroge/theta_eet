#include "theta_util.h"

int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a < b ? a : b; }

// Purpose:
//   ®”ˆÊ’u‚Ì‰æ‘f’lŽæ“¾
static void get_raw_pixel(const IplImage* in_img, const int u, const int v, CvScalar* col_bgr)
{
	if (0 <= u && u<in_img->width && 0 <= v && v < in_img->height) {
		col_bgr->val[COL_B] = (unsigned char)(in_img->imageData[in_img->widthStep*v + in_img->nChannels*u + 0]);
		col_bgr->val[COL_G] = (unsigned char)(in_img->imageData[in_img->widthStep*v + in_img->nChannels*u + 1]);
		col_bgr->val[COL_R] = (unsigned char)(in_img->imageData[in_img->widthStep*v + in_img->nChannels*u + 2]);
	}
	else {
		int uu = u;
		int vv = v;
		
		if (u < 0)
			uu = 0;
		else if (in_img->width <= u)
			uu = u % in_img->width;

		if (v < 0)
			vv = 0;
		else if (in_img->height <= v)
			vv = v % in_img->height;

		col_bgr->val[COL_B] = (unsigned char)(in_img->imageData[in_img->widthStep*vv + in_img->nChannels*uu + 0]);
		col_bgr->val[COL_G] = (unsigned char)(in_img->imageData[in_img->widthStep*vv + in_img->nChannels*uu + 1]);
		col_bgr->val[COL_R] = (unsigned char)(in_img->imageData[in_img->widthStep*vv + in_img->nChannels*uu + 2]);
	}
}

static void get_raw_pixel(const cv::Mat& in_img, const int u, const int v, cv::Scalar& col_bgr)
{
	if (0 <= u && u<in_img.cols && 0 <= v && v < in_img.rows) {
		for (int c = 0; c<3; c++) {
			col_bgr.val[c] = (unsigned char)(in_img.data[v*in_img.step + u*in_img.elemSize() + c]);
		}
	}
	else {
		int uu = u;
		int vv = v;

		if (u < 0)
			uu = 0;
		else if (in_img.cols <= u)
			uu = u % in_img.cols;

		if (v < 0)
			vv = 0;
		else if (in_img.rows <= v)
			vv = v % in_img.rows;

		for (int c = 0; c<3; c++) {
			col_bgr.val[c] = (unsigned char)(in_img.data[vv*in_img.step + uu*in_img.elemSize() + c]);
		}
	}
}

// Purpose:
//   ‰æ‘f•âŠÔƒ‚[ƒh‚É‡‚í‚¹‚Ä‰æ‘f’lŽæ“¾
void get_pixel(
	const IplImage* in_img, 
	const double ud, 
	const double vd, 
	CvScalar* col_bgr, 
	const int pix_inter_mode)
{
	int u = (int)ud;
	int v = (int)vd;
	CvScalar pix_t, pix_b, pix_l, pix_r;
	double ph_x = ud - u;
	double ph_y = vd - v;

	switch(pix_inter_mode) {
		case PIX_INTER_NONE:
			get_raw_pixel(in_img, u, v, col_bgr);
			break;
		case PIX_INTER_BILINER :
			get_raw_pixel(in_img, u, v - 1, &pix_t);
			get_raw_pixel(in_img, u, v + 1, &pix_b);
			get_raw_pixel(in_img, u - 1, v, &pix_l);
			get_raw_pixel(in_img, u + 1, v, &pix_r);

			for(int i=0; i < 3; i++) { // COL_B, COL_G, COL_R
				col_bgr->val[i] = 0.5 * (pix_t.val[i] * (1 - ph_y) + pix_b.val[i] * ph_y +
					pix_l.val[i] * (1 - ph_x) + pix_r.val[i] * ph_x);
			}
			break;
		default:
			break;
	}
}

void get_pixel(
	const cv::Mat& in_img,
	const double ud,
	const double vd,
	cv::Scalar& col_bgr,
	const int pix_inter_mode)
{
	int u = (int)ud;
	int v = (int)vd;
	cv::Scalar pix_t, pix_b, pix_l, pix_r;
	double ph_x = ud - u;
	double ph_y = vd - v;

	switch (pix_inter_mode) {
	case PIX_INTER_NONE:
		get_raw_pixel(in_img, u, v, col_bgr);
		break;
	case PIX_INTER_BILINER:
		get_raw_pixel(in_img, u, v - 1, pix_t);
		get_raw_pixel(in_img, u, v + 1, pix_b);
		get_raw_pixel(in_img, u - 1, v, pix_l);
		get_raw_pixel(in_img, u + 1, v, pix_r);

		for (int i = 0; i < 3; i++) { // COL_B, COL_G, COL_R
			col_bgr.val[i] = 0.5 * (pix_t.val[i] * (1 - ph_y) + pix_b.val[i] * ph_y +
				pix_l.val[i] * (1 - ph_x) + pix_r.val[i] * ph_x);
		}
		break;
	default:
		break;
	}
}

// Purpose: 
//	 Dual fisheye‹«ŠE•t‹ß‚ðalpha blending‚µ‚Ä‰æ‘f’lŽæ“¾
// Input:
//   dfe_img   : dual fisyeye image(dfe)
//   ud, vd    : dfe‚Ö‚ÌŽQÆ‰æ‘fˆÊ’u
//   cam1, cam2: camera1, camera2 î•ñ
//   which_cam : 0=cam1, 1=cam2
//   dfe_r     : ŽQÆ‰æ‘fˆÊ’u‚Ì”¼Œa
// Output:
//   col_bgr   : alpha blendingŒã‚Ì‰æ‘f’l
/*
void get_dfe_pixel(
	const IplImage* dfe_img,
	const double ud,
	const double vd,
	CvScalar* col_bgr,
	const THETA_USBCAM_CIRCLE_T* cam1,
	const THETA_USBCAM_CIRCLE_T* cam2,
	const int which_cam,	
	const double dfe_r
	)
{
	CvScalar pix_cam1, pix_cam2;
	double dr;	// alpha blending‹«ŠE‚©‚ç‚Ì”¼Œa•ûŒü‚Ì‹——£
	double rr;
	double k;	// alpha
	double t_ud, t_vd; // sub camera‚ÌŽQÆ‰æ‘fˆÊ’u
	double phi;

	if( which_cam==0 ) { // main camera is 'cam1'
		dr = dfe_r - cam1->r * DFE_ALPHA_BLENDING_R_RATIO;
		if ( dr <= 0 ) { // no alpha blending
			get_pixel(dfe_img, ud, vd, col_bgr);
		}
		else {
			k = 1 - 0.5 * dr / (cam1->r * (1 - DFE_ALPHA_BLENDING_R_RATIO));	// 0.5 <= k <= 1(dr=0‚Ì‚Æ‚«)B‹«ŠE‚Å0.5
			rr = cam2->r + (cam1->r * (1 - DFE_ALPHA_BLENDING_R_RATIO) - dr);
			phi = M_PI - atan2(vd - cam1->cpy, ud - cam1->cpx);
			t_ud = cam2->cpx + rr*cos(phi);
			t_vd = cam2->cpy + rr*sin(phi);
			get_pixel(dfe_img, ud, vd, &pix_cam1);
			get_pixel(dfe_img, t_ud, t_vd, &pix_cam2);

#ifdef _DEBUG
			printf("%6.2lf,%6.2lf,,%6.2lf,%6.2lf,%6.2lf,,%d,%6.2lf,%6.2lf,%6.2lf,%6.2lf,\n",
				ud, vd, cam1->cpx, cam1->cpy, cam1->r, which_cam, dfe_r, k, rr, phi/M_PI*180);
#endif

			for (int i = 0; i < 3; i++) { // COL_B, COL_G, COL_R
				col_bgr->val[i] = k * pix_cam1.val[i] + (1 - k) * pix_cam2.val[i];
			}
		}
	} 
	else { // main camera is 'cam2'
		dr = dfe_r - cam2->r * DFE_ALPHA_BLENDING_R_RATIO;
		if ( dr <= 0 ) { // no alpha blending
			get_pixel(dfe_img, ud, vd, col_bgr);
		} 
		else {
			k = 1 - 0.5 * dr / (cam2->r * (1 - DFE_ALPHA_BLENDING_R_RATIO));
			rr = cam1->r + (cam2->r * (1 - DFE_ALPHA_BLENDING_R_RATIO) - dr);
			phi = M_PI - atan2(vd - cam2->cpy, ud - cam2->cpx);
			t_ud = cam1->cpx + rr*cos(phi);
			t_vd = cam1->cpy + rr*sin(phi);
			get_pixel(dfe_img, t_ud, t_vd, &pix_cam1);
			get_pixel(dfe_img, ud, vd, &pix_cam2);

#ifdef _DEBUG
			printf("%6.2lf,%6.2lf,,%6.2lf,%6.2lf,%6.2lf,,%d,%6.2lf,%6.2lf,%6.2lf,%6.2lf,\n",
				ud, vd, cam2->cpx, cam2->cpy, cam2->r, which_cam, dfe_r, k, rr, phi / M_PI * 180);
#endif
			for (int i = 0; i < 3; i++) { // COL_B, COL_G, COL_R
				col_bgr->val[i] = (1 - k) * pix_cam1.val[i] + k * pix_cam2.val[i];
			}
		}
	}
	

}
*/

void set_pixel(IplImage* out_img, const int x, const int y, const CvScalar* col_bgr)
{
	if( 0<=x && x<out_img->width && 0<=y && y<out_img->height ) {
		out_img->imageData[out_img->widthStep*y + out_img->nChannels*x + 0] = (unsigned char)col_bgr->val[COL_B];
		out_img->imageData[out_img->widthStep*y + out_img->nChannels*x + 1] = (unsigned char)col_bgr->val[COL_G];
		out_img->imageData[out_img->widthStep*y + out_img->nChannels*x + 2] = (unsigned char)col_bgr->val[COL_R];
		out_img->imageData[out_img->widthStep*y + out_img->nChannels*x + 3] = 0;
	}
	else {
	}
}

void set_pixel(cv::Mat& out_img, const int x, const int y, const cv::Scalar& col_bgr)
{
	if (0 <= x && x<out_img.cols && 0 <= y && y<out_img.rows) {
		for (int c = 0; c < 3; c++) {
			out_img.data[y*out_img.step + x*out_img.elemSize() + c] = (unsigned char)col_bgr.val[c];
		}
	}
	else {
	}
}

double rad2deg(const double rad) { return rad / M_PI*180.0f; }
double deg2rad(const double deg) { return deg / 180.0f*M_PI; }

// lng,lat ‚Å•\Œ»‚³‚ê‚½“_‚ð sph_1 ‚ÌÀ•WŒn‚Å•\Œ»
//   sph_1(theta‰º‘¤‚ªYŽ²³)
void lnglat_to_sph(
	const double lng,
	const double lat,
	CvMat* sph_1 )
{
	cvmSet(sph_1, SPH_X, 0, cos(lat) * cos(lng));	// Z‚©‚çXŽ²•ûŒü‚ðŠp“x³
	cvmSet(sph_1, SPH_Y, 0, sin(lat));				// “V’¸Šp‚ÆˆÜ“x‚ÍŒü‚«‚ª‹t
	cvmSet(sph_1, SPH_Z, 0, -cos(lat) * sin(lng));
}

void lnglat_to_sph(
	const double lng,
	const double lat,
	cv::Mat& sph_1)
{
	sph_1.at<float>(SPH_X, 0) = cos(lat) * cos(lng);
	sph_1.at<float>(SPH_Y, 0) = sin(lat);
	sph_1.at<float>(SPH_Z, 0) = -cos(lat) * sin(lng);
}





