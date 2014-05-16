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


if __name__ == '__main__':
	# print len(sys.argv)
	in_file = sys.argv[1]
	out_file = 'out_file.txt'

	line_out = []
	with open(in_file) as in_f:
		with open(out_file, 'w') as out_f:
			for line in in_f:
				temp = line.split()
				line_out.extend([temp[1],temp[3],'\n'])
				line_out = ' '.join(line_out)
				out_f.write(line_out)
				line_out = []

