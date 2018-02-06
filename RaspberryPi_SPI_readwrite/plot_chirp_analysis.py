import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
##fig = plt.figure()
##ax = fig.add_subplot(111)
##a=[1,2,3,4,5];b=[13,4,-5,22,6];
##ax.plot(a,b)
##fig.savefig('/home/kalai/Desktop/temp.png')
##

fig1 = plt.figure()
ax1 = fig1.add_subplot(111)
zr=[];zi=[];
F=open('/home/pi/RPI_works/RPI_works/zr_demo.txt','r')
for row in F:
    if(row!='\n'):
        zr.append(float(row))
F.close()
F=open('/home/pi/RPI_works/RPI_works/zi_demo.txt','r')
for row in F:
    if(row!=' \n'):
        zi.append(float(row))
F.close()
plt.xlabel('real(z)');plt.ylabel('imag(z)')
ax1.plot(zr,zi);ax1.set_title('Chirp Analysis')
fig1.savefig('/home/pi/RPI_works/RPI_works/chirp_analysis.png')



i1=[];i2=[];
F=open('/home/pi/RPI_works/RPI_works/i1_demo.txt','r')
for row in F:
    if(row!='\n'):
        i1.append(float(row))
F.close()
F=open('/home/pi/RPI_works/RPI_works/i2_demo.txt','r')
for row in F:
    if(row!=' \n'):
        i2.append(float(row))
F.close()

fig2 = plt.figure();
plt.ylabel('Input signal');plt.xlabel('Time(t)')
ax_i1 = fig2.add_subplot(111)
ax_i1.plot(i1);ax_i1.set_title('i1')
fig2.savefig('/home/pi/RPI_works/RPI_works/i1.png')
fig3 = plt.figure();
plt.ylabel('Input signal');plt.xlabel('Time(t)')
ax_i2 = fig3.add_subplot(111)
ax_i2.plot(i2);ax_i2.set_title('i2')
fig3.savefig('/home/pi/RPI_works/RPI_works/i2.png')

v1=[];v2=[];
F=open('/home/pi/RPI_works/RPI_works/v1_demo.txt','r')
for row in F:
    if(row!='\n'):
        v=row.strip().split()
        v1.append(float(v[1]))
F.close()
v1=v1[0:len(v1)-1]
F=open('/home/pi/RPI_works/RPI_works/v2_demo.txt','r')
for row in F:
    if(row!=' \n'):
        v=row.strip().split()
        v2.append(float(v[1]))
F.close()
v2=v2[0:len(v2)-1]
fig4 = plt.figure();
plt.ylabel('Input signal');plt.xlabel('Time(t)')
ax_v1 = fig4.add_subplot(111)
ax_v1.plot(v1);ax_v1.set_title('v1')
fig4.savefig('/home/pi/RPI_works/RPI_works/v1.png')
fig5 = plt.figure();
plt.ylabel('Input signal');plt.xlabel('Time(t)')
ax_v2 = fig5.add_subplot(111)
ax_v2.plot(v2);ax_v2.set_title('v2')
fig5.savefig('/home/pi/RPI_works/RPI_works/v2.png')





