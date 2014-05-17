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
	out_file = 'out_file.txt'

	line_out = []
	with open(in_file) as in_f:
		with open(out_file, 'w') as out_f:
			for line in in_f:
				temp = line.split()
				line_out.extend([str(float(temp[0])*1e6),str(float(temp[1])*1e6),'\n'])
				line_out = ' '.join(line_out)
				out_f.write(line_out)
				line_out = []

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
