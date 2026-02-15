#ifndef AVS_H
#define AVS_H

typedef unsigned int        u32;

typedef enum
{
	AVS_OK = 0, // OK
	OUT_RANGE_FAIL, 	 // out of range FAIL
	TEMP_HIGH_FAIL		//temperature too high
} AVS_STATUS_T; // AVS status type

extern AVS_STATUS_T AVS_Set(u32 target_V);
extern u32 AVS_Get_Vcore(void);

#endif /* AVS_H */
