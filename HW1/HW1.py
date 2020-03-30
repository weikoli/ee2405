# Part. 1
#=======================================
# Import module
#  csv -- fileIO operation
import csv
#=======================================

# Part. 2
#=======================================
# Read cwb weather data
cwb_filename = 'sample_input.csv'
data = []
header = []

with open(cwb_filename) as csvfile:
   mycsv = csv.DictReader(csvfile)
   header = mycsv.fieldnames     
   for row in mycsv:
      data.append(row)
#=======================================

# Part. 3
#=======================================
# Analyze data depend on your group and store it to target_data like:
# Retrive all data points which station id is "C0X260" as a list.


t1 = list(filter(lambda item: item['station_id'] == 'C0A880', data))
t2 = list(filter(lambda item: item['station_id'] == 'C0F9A0', data))
t3 = list(filter(lambda item: item['station_id'] == 'C0G640', data))
t4 = list(filter(lambda item: item['station_id'] == 'C0R190', data))
t5 = list(filter(lambda item: item['station_id'] == 'C0X260', data))
output_C0A880 = []
output_C0F9A0 = []
output_C0G640 = []
output_C0R190 = []
output_C0X260 = []
output = []

for value in t1:
    if(value['WDSD'] != '-99.000') & (value['WDSD'] != '-999.000'):
        output_C0A880.append(float(value['WDSD']))

for value in t2:
    if(value['WDSD'] != '-99.000') & (value['WDSD'] != '-999.000'):
       output_C0F9A0.append(float(value['WDSD']))
        
for value in t3:
    if(value['WDSD'] != '-99.000') & (value['WDSD'] != '-999.000'):
        output_C0G640.append(float(value['WDSD']))
        
for value in t4:
    if(value['WDSD'] != '-99.000') & (value['WDSD'] != '-999.000'):
        output_C0R190.append(float(value['WDSD']))
        
for value in t5:
    if(value['WDSD'] != '-99.000') & (value['WDSD'] != '-999.000'):
        output_C0X260.append(float(value['WDSD']))
        

if len(output_C0A880) == 0:
    output.append(['C0A880', 'None'])
else:
    output.append(['C0A880', max(output_C0A880)-min(output_C0A880)])
if len(output_C0F9A0) == 0:
    output.append(['C0F9A0', 'None'])
else:
    output.append(['C0F9A0', max(output_C0F9A0)-min(output_C0F9A0)])
if len(output_C0G640) == 0:
    output.append(['C0G640', 'None'])
else:
    output.append(['C0G640', max(output_C0G640)-min(output_C0G640)])
if len(output_C0R190) != 0:
    output.append(['C0R190', 'None'])
else:
    output.append(['C0R190', max(output_C0R190)-min(output_C0R190)])
if len(output_C0X260) == 0:
    output.append(['C0X260', 'None'])
else:
    output.append(['C0X260', max(output_C0X260)-min(output_C0X260)])

#=======================================

# Part. 4
#=======================================
# Print result

print (output)

#========================================