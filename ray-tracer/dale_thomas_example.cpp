/*
  Dale Thomas's example from Quora:
  https://www.quora.com/Can-C++-be-used-to-create-graphics-Which-programming-language-should-be-learned-to-create-high-quality-graphics/answer/%E3%83%88%E3%83%BC%E3%83%9E%E3%82%B9-%E3%83%87%E3%83%BC%E3%83%AB-Dale-Thomas
*/

#include <stdio.h> 
#include <math.h> 
 
typedef struct{ 
	float x,y,z; 
}v;

const int S=1024,NS=5,NL=2; 
float T,Tm,A,sh; 
float s[][9]={{-12.73,-12.73,50,1,0,0,8,50,.3},{-12.73,12.73,50,0,1,0,8,50,.3},{12.73,12.73,50,0,0,1,8,50,.3},{12.73,-12.73,50,1,1,0,8,50,.3},{0,0,50,1,1,1,10,50,.6}}; 
float l[][7]={{-150,400,-20,.7,.7,.7,50},{350,100,-400,.4,.3,.35,50}}; 
 
float dot(v a,v b){ 
	return a.x*b.x+a.y*b.y+a.z*b.z; 
} 
float norm(v *V){ 
	T=sqrt(dot(*V,*V)); 
	V->x/=T,V->y/=T,V->z/=T; 
	return T; 
} 
int sect(int j,v o,v d,float &t){ 
	o.x-=s[j][0],o.y-=s[j][1],o.z-=s[j][2]; 
	t=-dot(o,d); 
	t-=sqrt(s[j][6]*s[j][6]-dot(o,o)+t*t); 
	if(t>.01) 
		return 1; 
	return 0; 
} 
v trace(v o,v d,int D){ 
	int h,i,j,k; 
	v c,n,L,J; 
	c.x=.3,c.y=.5,c.z=.8; 
	for(Tm=999,h=0,i=NS;i--; ) 
		if(sect(i,o,d,T)) 
			if(T<Tm) 
				Tm=T,h=i+1; 
	if(h--){ 
		o.x+=d.x*Tm,o.y+=d.y*Tm,o.z+=d.z*Tm; 
		n.x=o.x-s[h][0],n.y=o.y-s[h][1],n.z=o.z-s[h][2],norm(&n); 
		T=dot(n,d)*2,d.x-=n.x*T,d.y-=n.y*T,d.z-=n.z*T; 
		if(D--) 
			c=trace(o,d,D); 
		c.x*=s[h][8],c.y*=s[h][8],c.z*=s[h][8]; 
		c.x+=.1,c.y+=.1,c.z+=.1; 
		for(j=NL;j--; ){ 
			L.x=l[j][0]-o.x,L.y=l[j][1]-o.y,L.z=l[j][2]-o.z,A=norm(&L); 
			for(sh=1,i=NS;i--; ){ 
				J.x=s[i][0]-o.x,J.y=s[i][1]-o.y,J.z=s[i][2]-o.z; 
				T=dot(L,J); 
				if(T>0&&T<A){ 
					J.x-=L.x*T,J.y-=L.y*T,J.z-=L.z*T,T=l[j][6]*T/A; 
					T=(sqrt(dot(J,J))-s[i][6]+T)/(T+T); 
					T=(T<0)?0:(T>1)?1:T; 
					sh*=T; 
				} 
			} 
			T=dot(L,n),T=(T>0)?T*(1-s[h][8])*sh:0; 
			c.x+=l[j][3]*s[h][3]*T,c.y+=l[j][4]*s[h][4]*T,c.z+=l[j][5]*s[h][5]*T; 
			T=dot(L,d),T=(T>0)?pow(T,s[h][7]):0; 
			T*=sh; 
			c.x+=l[j][3]*T,c.y+=l[j][4]*T,c.z+=l[j][5]*T; 
		} 
	} 
	return c; 
} 
int main(){ 
	char P[3]; 
	v o,d,c; 
	short h[]={0,2,0,0,0,0,S,S,24}; 
	FILE *f=fopen("1.tga","wb"); 
	fwrite(h,18,1,f); 
	for(int i=S*S;i--; ){ 
		o.x=o.y=o.z=0,d.x=S/2-i%S,d.y=S/2-i/S,d.z=S,norm(&d); 
		c=trace(o,d,5); 
		P[0]=(c.z>1)?255:(char)(c.z*255),P[1]=(c.y>1)?255:(char)(c.y*255),P[2]=(c.x>1)?255:(char)(c.x*255); 
		fwrite(&P,3,1,f); 
		if(!(i%S)) 
			printf("%d ",i/S); 
	} 
	fclose(f); 
	return 0; 
} 
