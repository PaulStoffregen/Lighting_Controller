#  This script parses a Vixen Sequence file and outputs a text
#  file that can be easily played from a microcontroller.
#
#  Vixen version 2.1.1.0 must be used to create the file.
#
#  Based on Bill's Super Awesome Vixen to Arduino/AVR/C script
#  (www.billporter.info) with several modifications...
#  http://www.billporter.info/2012/10/07/tutorial-vixeno-vixen-sequence-conversion-for-arduino/
#
#  Run it like this:
#      path> python Vixen2core.py
#

filein =  'test1.vix'
fileout = 'test1.txt'

import base64
import sys
import xml.etree.ElementTree as ET

# read the .vix file and extract the necessary stuff
print ('Opening %s ' % filein)
doc = ET.parse(filein)
root = doc.getroot()
period = root.find('EventPeriodInMilliseconds').text
channeltop = root.find('Channels')
channel = channeltop.findall('Channel')
channels = len(channel)
data = root.find('EventValues').text

# decode vixen's data
vixenbinary = base64.b64decode(data)
outputarray = list(vixenbinary)

# convert binary elements to decimal
outputarray = [ord(c) for c in outputarray]

#figure out number of samples in the show
samples = len(outputarray) / channels

print('Found %d Channels and %d frames for %d total bytes of memory' % (channels, samples, channels*samples))

# start creating output file
fileout = open(fileout, 'w')
fileout.write('%d\n' % int(period))
for n in range(samples):
	for ch in range(channels):
		fileout.write('%02X' % outputarray[ch*samples + n])
	fileout.write('\n')
fileout.close()


