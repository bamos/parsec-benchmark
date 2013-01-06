

#include "RTTL/common/RTVec.hxx"
#include "RTTL/common/Timer.hxx"

using RTTL::vec3f;
using RTTL::vec3fa;

int main()
{
  vec3f u(3,2,1);
  vec3fa a(3,2,1);

  cout << "alignenment u " << (int*)&u << endl;
  cout << "alignenment a " << (int*)&a << endl;
  float *f = NULL;
  
  for (int i=0;i<100000;i++)
    {
      a = a + a;
      *(vec3fa *)f = a;
    }
  for (int i=0;i<100000;i++)
    {
      u = u + u;
    }

  cout << "not aligned " << u << endl;
  cout << "is aligned " << a << endl;
};
