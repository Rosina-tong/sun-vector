import sys
import time as timer
import math
import datetime
import string
import numpy as np
from numpy import linalg as lg
import configparser
import sun_vector as solar

def sol_test(config_fil):

	ARG_LEN = 20
	i = 0

	wind_ld = 9000

	#initialize variable, lists

	tm = []
	fd_arg = []
	spa_arg = ''


	#read in configurations
	start_time = sec_time = timer.time()
	print('Reading Configurations...')

	config = configparser.ConfigParser()
	config.optionxform = str
	config.read(config_fil)


	#read in general configurations

	cfg_sect = {}
	cfg_sect = ConfigSectionMap('general', config, 0)

	longitude = cfg_sect.pop('longitude')
	latitude = cfg_sect.pop('latitude')
	s_year = cfg_sect.pop('s_year')
	s_month = cfg_sect.pop('s_month')
	s_day = cfg_sect.pop('s_day')
	s_hour = cfg_sect.pop('s_hour')
	s_minute = cfg_sect.pop('s_minute')
	s_second = cfg_sect.pop('s_second')
	e_year = cfg_sect.pop('e_year')
	e_month = cfg_sect.pop('e_month')
	e_day = cfg_sect.pop('e_day')
	e_hour = cfg_sect.pop('e_hour')
	e_minute = cfg_sect.pop('e_minute')
	e_second = cfg_sect.pop('e_second')
	timezone = cfg_sect.pop('timezone')
	elevation = cfg_sect.pop('elevation')
	pressure = cfg_sect.pop('pressure')
	temperature = cfg_sect.pop('temperature')
	delta_t = cfg_sect.pop('delta_t')
	tstep = cfg_sect.pop('tstep')



	print('Done', timer.time()-sec_time, 'Sec')

	#get solar position
	print('\nCalculating solar position...', end="")
	sec_time = timer.time()

	spa_arg = longitude+' '+latitude+' '+s_year+' '+s_month+' '+s_day+' '+s_hour+' '+s_minute+' '+s_second+' '+e_year+' '+e_month+' '+e_day+' '+e_hour+ \
		' '+e_minute+' '+e_second+' '+timezone+' '+elevation+' '+pressure+' '+temperature+' '+delta_t+' '+tstep

	a = np.mat([])

	a.astype(np.float64)

	print(solar.sun_pos.__doc__)
	a = solar.sun_pos(spa_arg)

	a = np.squeeze(np.asarray(a))
	a = np.transpose(a)

	print(a)

	spa_arg = longitude+' '+latitude+' '+s_year+' '+s_month+' '+s_day+' '+timezone+' '+elevation+' '+pressure+' '+temperature+' '+delta_t+' '+tstep

	print(solar.sun_pos_day.__doc__)
	a = solar.sun_pos_day(spa_arg)

	a = np.squeeze(np.asarray(a))
	a = np.transpose(a)

	print(a)

	print('Done\t', timer.time()-sec_time, 'Sec\n')

def ConfigSectionMap(section, cfg, flag):

	dict1 = {}
	options = cfg.options(section)

	if flag:
		for option in options:
			try:
				dict1[option] = cfg.getfloat(section, option)
				if dict1[option] == -1:
					DebugPrint("skip: %s" % option)
			except:
				try:
					dict1[option] = cfg.getint(section, option)
					if dist1[option] == -1:
						DebugPrint("skip: %s" % option)
				except:
					try:
						dict1[option] = cfg.get(section, option)
						if dict1[option] == -1:
							DebugPrint("skip: %s" % option)
					except:
						print("exception on %s!" % option)
						dict1[option] = None
	elif not flag:
		for option in options:
			try:
				dict1[option] = cfg.get(section, option)
				if dict1[option] == -1:
					DebugPrint("skip: %s" % option)
			except:
				print("exception on %s!" % option)
				dict1[option] = None


	return(dict1)
