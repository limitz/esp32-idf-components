#include "jpg_decode.h"

#include <esp32/rom/tjpgd.h>
#define WORKSPACE 4096

int jpg_decode()
{
	int err;

	*result = (uint16_t*) calloc(h * w, sizeof(uint16_t));
	if (!*result) 
	{
		return ESP_ERR_NO_MEM;
	}

	void* work = calloc(WORKSPACE, 1);
	if (!work) 
	{
		free(result);
		return ESP_ERR_NO_MEM;
	}

	jpg_dev_t dev = {
		.in.data = data,
		.in.pos = 0,
		.out.data = result,
		.out.w = w,
		.out.h = h,
	};

	JDEC decoder;
	err = jd_prepare(&decoder, in_cb, work, WORKSPACE, &dev);
	if (JDR_OK != err) 
	{
		free(result);
		free(work);
		return ESP_ERR_NOT_SUPPORTED;
	}

	err = jd_decomp(&decder, out_cb, 0);
	if (JDR_OK != err)
	{
		free(result);
		free(work);
		return ESP_ERR_NOT_SUPPORTED;
	}

	free(work);
	return ESP_OK;
}
