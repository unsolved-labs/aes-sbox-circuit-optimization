#include <bits/stdc++.h>
using namespace std; using U=unsigned __int128;
static int msb(U x){uint64_t hi=(uint64_t)(x>>64); if(hi) return 64+63-__builtin_clzll(hi); uint64_t lo=(uint64_t)x; return lo?63-__builtin_clzll(lo):-1;}
static U ph(const string&s){U x=0;for(char c:s){int v;if(c>='0'&&c<='9')v=c-'0';else if(c>='a'&&c<='f')v=c-'a'+10;else if(c>='A'&&c<='F')v=c-'A'+10;else continue;x=(x<<4)|v;}return x;}
struct E{U v[66]{}; uint32_t c[66]{}; int r=0; bool add(U x,uint32_t co=0){while(x){int p=msb(x);if(v[p]){x^=v[p];co^=c[p];}else{v[p]=x;c[p]=co;r++;return true;}}return false;} bool coords(U x,uint32_t &co)const{co=0;for(int p=65;p>=0;p--)if(v[p]&&((x>>p)&1)){x^=v[p];co^=c[p];}return x==0;}};
static int r32(const vector<uint32_t>&xs){uint32_t b[32]{};int r=0;for(uint32_t x:xs){while(x){int p=31-__builtin_clz(x);if(b[p])x^=b[p];else{b[p]=x;r++;break;}}}return r;}
int main(){
const char* rows=R"WIT(A 128c
0c0 00c 00000000303000000
080 008 00000000200000000
040 004 00000000001000000
84c cc8 20848001233200000
848 cc4 20848003321200000
448 c8c 204c8002311200000
8c8 c84 204c8003223200000
4c8 c4c 20c88002213200000
020 002 00000000000004000
010 001 00000000000000008
030 003 00000000000006018
132 221 01001844000044099
131 223 01001844000026189
331 212 01001024000066109
121 213 01001024000024199
321 232 01000864000064119
0e0 00e 0000000038381c000
090 009 00000000240000048
070 007 00000000001c0e038
97e ee9 3797bddde7bbdd4bf
979 ee7 3797bddfba5f2f3cf
779 e9e 377ef53e79deef74f
9e9 e97 377ef53fa67b3d3bf
7e9 e7e 37e9ccfe65fafd73f
COEFF
030606
030500
050303
060300
97481f
97b8b8
b83057
67efb8
6748ef
300030
30df00
67df57)WIT";
 istringstream in(rows); string a,b,c; in>>a>>b; vector<U> p; vector<uint32_t> q; while(in>>a){if(a=="COEFF")break;in>>b>>c;p.push_back(ph(c));}while(in>>a)q.push_back(stoul(a,nullptr,16)); if(p.size()!=24||q.size()!=12)return 2;
 int pi[12][12]; memset(pi,-1,sizeof(pi)); int kk=0; for(int i=0;i<12;i++)for(int j=i+1;j<12;j++)pi[i][j]=kk++;
 static U wb[4096][12]; for(int u=0;u<4096;u++)for(int j=0;j<12;j++){U w=0;for(int i=0;i<12;i++)if((u>>i&1)&&i!=j)w^=U(1)<<pi[min(i,j)][max(i,j)];wb[u][j]=w;}
 auto wedge=[&](uint16_t u,uint16_t v){U w=0;while(v){int j=__builtin_ctz(v);v&=v-1;w^=wb[u][j];}return w;};
 E P; for(int i=0;i<24;i++) if(!P.add(p[i],1u<<i)) return 3; if(P.r!=24)return 4;
 vector<uint32_t> dc; dc.reserve(32); uint64_t total=0;
 for(uint16_t u=1;u<4096;u++)for(uint16_t v=u+1;v<4096;v++){uint16_t wv=u^v;if(!(v<wv))continue;total++;uint32_t co;if(P.coords(wedge(u,v),co))dc.push_back(co);} sort(dc.begin(),dc.end()); dc.erase(unique(dc.begin(),dc.end()),dc.end());
 uint32_t rr[12]; for(int i=0;i<12;i++)rr[i]=q[i]; vector<int> piv; int r=0; for(int col=23;col>=0&&r<12;col--){int sel=-1;for(int i=r;i<12;i++)if(rr[i]>>col&1){sel=i;break;}if(sel<0)continue;swap(rr[r],rr[sel]);for(int i=0;i<12;i++)if(i!=r&&(rr[i]>>col&1))rr[i]^=rr[r];piv.push_back(col);r++;} if(r!=12)return 5; bool isp[24]{};for(int x:piv)isp[x]=1; vector<int> freec;for(int i=0;i<24;i++)if(!isp[i])freec.push_back(i);vector<uint32_t> nb;for(int f:freec){uint32_t x=1u<<f;for(int i=0;i<12;i++)if(rr[i]>>f&1)x|=1u<<piv[i];nb.push_back(x);} if(nb.size()!=12)return 6;
 map<int,int> hist; int best=0; for(uint32_t m=1;m<(1u<<12);m++){uint32_t l=0;for(int i=0;i<12;i++)if(m>>i&1)l^=nb[i];vector<uint32_t> inside;for(uint32_t x:dc)if((__builtin_parity(x&l)==0))inside.push_back(x);int z=r32(inside);hist[z]++;best=max(best,z);} 
 cout<<"canonical decomposable bivectors: "<<total<<"\n"; cout<<"decomposable points in published P: "<<dc.size()<<"\n"; cout<<"target rank: 12\n"; cout<<"target-containing hyperplanes checked: 4095\n"; cout<<"decomposable-span rank histogram:";for(auto [k,v]:hist)cout<<" "<<k<<":"<<v;cout<<"\n";cout<<"maximum decomposable span rank: "<<best<<"\n"; if(total!=2794155||dc.size()!=27||best!=21)return 7; cout<<"PASS: no 23-product outer realization exists entirely inside the published 24-product span.\n";return 0;
}
