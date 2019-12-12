
import string
import random
from random import randint
filenames= ['Zhuo','Ling','Chen']


#define random function
#learning from:http://mp.weixin.qq.com/mp/appmsg/show?__biz=MjM5MDEyMDk4Mw==&appmsgid=10000086&itemidx=1&sign=e825894cfb9008b0ea78e81e5d930969
#for loop to pick from 1 to 10 letters
#learning from :http://mp.weixin.qq.com/mp/appmsg/show?__biz=MjM5MDEyMDk4Mw==&appmsgid=10000061&itemidx=1&sign=c82ada59e5bfdb2edbffa6c959c73e69
def Letterand():
  	Letter = ""
	for i in range(10):
        	Letter  += (random.choice(string.ascii_lowercase))
  	return Letter
#read file
#learning from http://mp.weixin.qq.com/mp/appmsg/show?__biz=MjM5MDEyMDk4Mw==&appmsgid=10000139&itemidx=1&sign=a41863d00cc01c9953ac9377838c5cfb
for i in range(3):
	f = open(filenames[i],"w")
	f.write(Letterand());
	f.close()

for i in range(3):
        f = open(filenames[i],"r")
        print f.read()
        f.close()

#print a,b which range between 1-42 
#c is a mutiply b
#learning from:http://mp.weixin.qq.com/mp/appmsg/show?__biz=MjM5MDEyMDk4Mw==&appmsgid=10000035&itemidx=1&sign=3fb61a6d8130c402736114c1443d217d
a = randint(1,42)
b = randint(1,42)
c = a * b
#print a b c
print(a)
print(b)
print(c)
