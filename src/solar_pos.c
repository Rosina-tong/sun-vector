#include <python3.4/Python.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "spa.h"  //include the SPA header file

#define ARG_NUM 20

#define PI 3.1415926

static char docstring_pos[] = "\nThis module provides a vector containing specificed time frame in float\nhours, solar zenit in radians, and solar azimuth in radians for over\nspecified timer interval.\n\nUsage: sun_vector.sun_pos(Longitude, Latitude, s_Year, s_Month, s_Day, s_Hour,\ns_Minute, s_Second, e_Year, e_Month, e_Day, e_Hour,\ne_Minute, e_Second, Time zone, Elevation, Pressure,\nTemperature, Delta_t, Time step)\n\n";

static char docstring_pos_day[] = "\nThis module provides a vector containing specificed time frame in float\nhours, solar zenit in radians, and solar azimuth in radians for from\nsunrise to sunsite for the specified day.\n\nUsage: sun_vector.sun_pos(Longitude, Latitude, Year, Month, Day, Time zone,\nElevation, Pressure, Temperature, Delta_t,\nTime step)\n\n";

//usage: sun_vector.sun_pos(Long, Lat, Year, Month, Day, Time_Z, Elev, Pres, Temp, Delta_T, T_Step)

static PyObject *sun_pos(PyObject *self, PyObject *args){

	spa_data spa;  //declare the SPA structure
	int result, hr_rise, min_rise, sec_rise, hr_set, min_set, sec_set, i,*tm, year_fin, month_fin, day_fin, hour_fin, min_fin, sec_fin, tm_len, t_stp;
	float min, sec, *flt_arg;
	double *z_ang, *az_ang;
	char *token, *argmts;
	const char q[3] = " ";
	PyObject *py_res, *py_tm, *py_az, *py_z, *num;
	struct tm tm_comp;
	time_t tm_i, tm_fin;
	clock_t beg, end;



	flt_arg = malloc(ARG_NUM*sizeof(float));

	//get arguments from calling python program

	if(!PyArg_ParseTuple(args, "s", &argmts))
		return NULL;


	token = strtok(argmts, q);

	i=0;

	while(token!=NULL){
		*(flt_arg+i) = atof(token);
		token = strtok(NULL, q);
		i++;
	}


	//enter required input values into SPA structure

	spa.year          = (int)flt_arg[2];
	spa.month         = (int)flt_arg[3];
	spa.day           = (int)flt_arg[4];
	spa.timezone      = (int)flt_arg[14];
	spa.delta_ut1     = 0;

	//delta_t = 32.184 + (TAI-UTC) + DUT1 (from http://maia.usno.navy.mil/ser7/ser7.dat)
	spa.delta_t       = flt_arg[18];
	spa.longitude     = flt_arg[0];
	spa.latitude      = flt_arg[1];
	spa.elevation     = flt_arg[15];
	spa.pressure      = flt_arg[16];
	spa.temperature   = flt_arg[17];
	spa.slope         = 0;
	spa.azm_rotation  = 0;
	spa.atmos_refract = 0.5667;
	spa.hour 		= flt_arg[5];
	spa.minute 		= flt_arg[6];
	spa.second 		= flt_arg[7];
	spa.function 		= SPA_ALL;

	year_fin 	= flt_arg[8];
	month_fin 	= flt_arg[9]-1;
	day_fin		= flt_arg[10];
	hour_fin	= flt_arg[11];
	min_fin		= flt_arg[12];
	sec_fin		= flt_arg[13];	

	t_stp = flt_arg[19];


	i = 0;

	tm_comp.tm_year = year_fin-1900;
	tm_comp.tm_mon = month_fin;
	tm_comp.tm_mday = day_fin;
	tm_comp.tm_hour = hour_fin;
	tm_comp.tm_min = min_fin;
	tm_comp.tm_sec = sec_fin;

	tm_fin = mktime(&tm_comp);

	tm_comp.tm_year = spa.year-1900;
	tm_comp.tm_mon = spa.month-1;
	tm_comp.tm_mday = spa.day;
	tm_comp.tm_hour = spa.hour;
	tm_comp.tm_min = spa.minute;
	tm_comp.tm_sec = spa.second;

	tm_i = mktime(&tm_comp);

	printf("\nstart time: %s\n", ctime(&tm_i));
	printf("end time: %s\n", ctime(&tm_fin));

	tm_len = ceil((tm_fin-tm_i)/t_stp);

	tm = malloc(tm_len*sizeof(int));
	z_ang = malloc(tm_len*sizeof(double));
	az_ang = malloc(tm_len*sizeof(double));

	//calculate sunrise and sun set

	result = spa_calculate(&spa);

	if (result == 0){

		hr_rise = (int)(spa.sunrise);
	        min = 60.0*(spa.sunrise - (int)(spa.sunrise));
	        sec = 60.0*(min - (int)min);

		min_rise = (int)min;
		sec_rise = (int)sec;

	        printf("Sunrise: %02d:%02d:%02d Local Time\n", hr_rise, min_rise, sec_rise);

		hr_set = (int)(spa.sunset);
	        min = 60.0*(spa.sunset - (int)(spa.sunset));
	        sec = 60.0*(min - (int)min);

		min_set = (int)min;
		sec_set = (int)sec;

	        printf("Sunset: %02d:%02d:%02d Local Time\n\n", hr_set, min_set, sec_set);
	}
	else{
		printf("SPA Error Code: %d\n", result);
		exit(1);
	}

	py_res = PyList_New(3);

	i = 0;

	beg = clock();

	py_tm = PyList_New(tm_len);
	py_z = PyList_New(tm_len);
	py_az = PyList_New(tm_len);

	i = tm_len;

	for(i=0; i<tm_len; i++){

		spa.year	= tm_comp.tm_year+1900;
		spa.month	= tm_comp.tm_mon+1;
		spa.day		= tm_comp.tm_mday;
		spa.hour	= tm_comp.tm_hour;
		spa.minute	= tm_comp.tm_min;
		spa.second	= tm_comp.tm_sec;

		result = spa_calculate(&spa);

		*(z_ang+i) = spa.zenith;
		*(az_ang+i) = spa.azimuth180;
		*(tm+i) = mktime(&tm_comp);


		num = PyFloat_FromDouble(*(tm+i));
		PyList_SET_ITEM(py_tm, i, num);

		num = PyFloat_FromDouble(PI**(z_ang+i)/180);
		PyList_SET_ITEM(py_z, i, num);

		num = PyFloat_FromDouble(PI**(az_ang+i)/180);
		PyList_SET_ITEM(py_az, i, num);

		tm_comp.tm_sec += t_stp;
		mktime(&tm_comp);
    	}

	printf("finished computing vector\n");


	PyList_SET_ITEM(py_res, 0, py_tm);
	PyList_SET_ITEM(py_res, 1, py_z);
	PyList_SET_ITEM(py_res, 2, py_az);

	free(z_ang);
	free(az_ang);
	free(tm);

	fflush(stdout);
	end = clock();

	printf("Solar Vector execution time: %f sec\n", (float)(end-beg)/CLOCKS_PER_SEC);

	return(py_res);
}

//usage: sun_vector.sun_pos_day(Long, Lat, Year, Month, Day, Time_Z, Elev, Pres, Temp, Delta_T, T_Step)

static PyObject *sun_pos_day(PyObject *self, PyObject *args){

	spa_data spa;  //declare the SPA structure
	int result, hr_rise, min_rise, sec_rise, hr_set, min_set, sec_set, i, j, tm_len;
	float min, sec, *flt_arg, t, t_stp, *tm;
	double *z_ang, *az_ang;
	char *token, *argmts;
	const char q[3] = " ";
	PyObject *py_res, *py_tm, *py_az, *py_z, *num;
	clock_t beg, end;


	flt_arg = malloc(ARG_NUM*sizeof(float));

	//get arguments from calling python program

	if(!PyArg_ParseTuple(args, "s", &argmts))
		return NULL;


	token = strtok(argmts, q);

	i=0;

	while(token!=NULL){
		*(flt_arg+i) = atof(token);
		token = strtok(NULL, q);
		i++;
	}

	//enter required input values into SPA structure

	spa.year          = (int)flt_arg[2];
	spa.month         = (int)flt_arg[3];
	spa.day           = (int)flt_arg[4];
	spa.timezone      = (int)flt_arg[5];
	spa.delta_ut1     = 0;

	//delta_t = 32.184 + (TAI-UTC) + DUT1 (from http://maia.usno.navy.mil/ser7/ser7.dat)
	spa.delta_t       = flt_arg[9];
	spa.longitude     = flt_arg[0];
	spa.latitude      = flt_arg[1];
	spa.elevation     = flt_arg[6];
	spa.pressure      = flt_arg[7];
	spa.temperature   = flt_arg[8];
	spa.slope         = 0;
	spa.azm_rotation  = 0;
	spa.atmos_refract = 0.5667;
	spa.hour 		= 0;
	spa.minute 		= 0;
	spa.second 		= 0;
	spa.function 		= SPA_ALL;

	t_stp = flt_arg[10]/3600;

	i = 0;

	//calculate sunrise and sun set

	result = spa_calculate(&spa);

	if (result == 0){

		hr_rise = (int)(spa.sunrise);
	        min = 60.0*(spa.sunrise - (int)(spa.sunrise));
	        sec = 60.0*(min - (int)min);

		min_rise = (int)min;
		sec_rise = (int)sec;

	        printf("Sunrise: %02d:%02d:%02d Local Time\n", hr_rise, min_rise, sec_rise);

		hr_set = (int)(spa.sunset);
	        min = 60.0*(spa.sunset - (int)(spa.sunset));
	        sec = 60.0*(min - (int)min);

		min_set = (int)min;
		sec_set = (int)sec;

	        printf("Sunset: %02d:%02d:%02d Local Time\n\n", hr_set, min_set, sec_set);
	}
	else{
		printf("SPA Error Code: %d\n", result);
		exit(1);
	}

	tm_len = ceil((spa.sunset-spa.sunrise)/(t_stp));

	tm = malloc(tm_len*sizeof(float));
	z_ang = malloc(tm_len*sizeof(double));
	az_ang = malloc(tm_len*sizeof(double));

	py_res = PyList_New(3);

	beg = clock();

	i = 0;

	for(t = spa.sunrise; t<=spa.sunset; t+=t_stp){

		min = 60.0*(t - (int)t);
		sec = 60.0*(min - (int)min);

		spa.hour	= (int)t;
		spa.minute	= (int)min;
		spa.second	= (int)sec;


		result = spa_calculate(&spa);

		*(z_ang+i) = spa.zenith;
		*(az_ang+i) = spa.azimuth180;

		if(result == 0){
			if(*(z_ang+i) >= 0.0 && *(z_ang+i) <=90.0 && *(az_ang+i) >= -180.0 && *(az_ang+i) <=180.0 ){
				*(tm+i) = t;

				i++;
			}
		}
		else if(result != 0){
			printf("SPA Error Code: %d\n", result);
			exit(1);
		}
	}


	z_ang = realloc(z_ang, (i)*sizeof(double));
	az_ang = realloc(az_ang, (i)*sizeof(double));
	tm = realloc(tm, (i)*sizeof(float));

	py_tm = PyList_New(i);
	py_z = PyList_New(i);
	py_az = PyList_New(i);

	for(j=0; j<i; j++){
		num = PyFloat_FromDouble(*(tm+j));
		PyList_SET_ITEM(py_tm, j, num);
		
		num = PyFloat_FromDouble(PI**(z_ang+j)/180);
		PyList_SET_ITEM(py_z, j, num);

		num = PyFloat_FromDouble(PI**(az_ang+j)/180);
		PyList_SET_ITEM(py_az, j, num);
    	}


	printf("finished computing vector\n");

	PyList_SET_ITEM(py_res, 0, py_tm);
	PyList_SET_ITEM(py_res, 1, py_z);
	PyList_SET_ITEM(py_res, 2, py_az);

	free(z_ang);
	free(az_ang);
	free(tm);

	fflush(stdout);
	end = clock();

	printf("Daily Solar vector execution time: %f sec\n", (float)(end-beg)/CLOCKS_PER_SEC);

	return(py_res);
}

static PyMethodDef sun_method[] = {{"sun_pos", sun_pos, METH_VARARGS, docstring_pos},
				{"sun_pos_day", sun_pos_day, METH_VARARGS, docstring_pos_day}, 
				{NULL, NULL, 0, NULL}};

static struct PyModuleDef sun_module = {PyModuleDef_HEAD_INIT, "SUN POSITION", NULL, -1, sun_method};

PyMODINIT_FUNC PyInit_sun_vector(void){

	PyObject *m;

	m = PyModule_Create(&sun_module);
	if(m==NULL)
		return NULL;

	return m;
}
