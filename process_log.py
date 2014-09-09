# from numpy import *
# import threading
# import matplotlib.pyplot as plt
# import time
# import wx
# import wx.lib.plot as plot
# from wx.lib.pubsub import Publisher as pub
# import numpy as np
# import os
import sys 
from math import pow


if __name__ == '__main__':
	# print len(sys.argv)
	in_file = sys.argv[1]
	out_file = sys.argv[2]


	# line_out = []
	# inc = 5
	# with open(in_file, 'w') as in_f:

	# 	for i in range(200):
	# 		line_out.extend([str(1), str(inc)])
	# 		line_out = ' '.join(line_out)
	# 		line_out += '\n'
	# 		in_f.write(line_out)
	# 		line_out = []
	# 		inc += 5



	line_out = []
	with open(in_file) as in_f:
		with open(out_file, 'w') as out_f:
			for line in in_f:
				temp = line.split()
				# line_out.extend([str(float(temp[1])*1e-6/60),str(float(temp[3])*1e-3*1e1),'\n'])
				line_out.extend([str(float(temp[1])),str(float(temp[3])),'\n'])
				line_out = ' '.join(line_out)
				out_f.write(line_out)
				line_out = []


	# temp_inter = 0	
	# cur = 0
	# cur_prev = 0		
	# line_out = []
	# with open(in_file) as in_f:
	# 	with open(out_file, 'w') as out_f:
	# 		for line in in_f:
	# 			temp = line.split()
	# 			cur = float(temp[3])*1e-3
	# 			inter = float(temp[1])*1e-6/60
	# 			if abs(cur - cur_prev) >1:
	# 				line_out.extend([str(temp_inter), str(cur_prev), '\n'])
	# 				line_out = ' '.join(line_out)
	# 				out_f.write(line_out)
	# 				line_out = []
	# 				temp_inter = inter; 
	# 			else : 
	# 				temp_inter += inter
	# 			cur_prev = cur


	# line_out = []
	# out_file = 'new_log.txt'
	# with open(in_file) as in_f:
	# 	with open(out_file, 'w') as out_f:
	# 		for line in in_f:
	# 			temp = line.split()
	# 			newstr = temp[3].replace(",", "")
	# 			line_out.extend([str(float(temp[1])),str(float(newstr)),'\n'])
	# 			line_out = ' '.join(line_out)
	# 			out_f.write(line_out)
	# 			line_out = []



	# act_time = 0
	# act_n = 0
	# slp_time = 0
	# slp_n = 0
	# with open(in_file) as in_f:
	# 	for line in in_f:
	# 		temp = line.split()
	# 		if float(temp[1]) > 70000:
	# 			continue
	# 		if temp[0] == "ACT_Int":
	# 			act_time = act_time + float(temp[1])
	# 			act_n = act_n + 1
	# 		elif temp[0] == "SLP_Int":
	# 			slp_time = slp_time + float(temp[1])
	# 			slp_n = slp_n + 1			

	# act_mean = act_time / act_n
	# slp_mean = slp_time / slp_n




	# act_mean = act_time / act_n
	# slp_mean = slp_time / slp_n

	# print 'ave act step = ', act_mean, '\nave slp step = ', slp_mean

	# line_out = []
	# with open(in_file) as in_f:
	# 	with open(out_file, 'w') as out_f:
	# 		alpha = float(in_f.readline())
	# 		alpha_as = alpha * 1e-3 * 60 * 1e6 * 1e6
	# 		out_f.write(str(alpha_as)+'\n')
	# 		print 'alpha = ', alpha, ' alpha_as = ', alpha_as

	# 		beta = float(in_f.readline())
	# 		beta_s = beta * pow(60*1e6,-0.5)
	# 		out_f.write(str(beta_s)+'\n')
	# 		print 'beta = ', beta, ' beta_s = ', beta_s

	# 		numterms = int(in_f.readline())
	# 		out_f.write(str(numterms)+'\n')
	# 		print 'numterms = ', numterms

	# 		delta = float(in_f.readline())
	# 		out_f.write(str(1)+'\n')
	# 		print 'delta = ', delta, ' delta_s = 1'

	# time = 0
	# with open(in_file) as in_f:	
	# 	for line in in_f:
	# 		temp = line.split()
	# 		time = time + float(temp[0])


	# print 'total time = ', time/1e6, ' s\n'






