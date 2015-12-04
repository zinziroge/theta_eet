#ifndef __THETA_EET_H__
#define __THETA_EET_H__

#define		ROT_PER_SEC	(-360.0f/24.0/3600.0/180.0*M_PI)	// [rad]

typedef struct {
	float np_x;
	float np_y;
	float rot_vec[3];	// rotation unit vector
	float interval_time;
} THETA_EET_CFG_T;

#endif