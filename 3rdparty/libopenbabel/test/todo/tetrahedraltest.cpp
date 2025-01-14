#include <boost/test/unit_test.hpp>
#include <openbabel/stereo/tetrahedral.h>

using namespace std;
using namespace OpenBabel;

bool sameWinding(const OBStereo::Refs &refs1, const OBStereo::Refs &refs2)
{
  BOOST_REQUIRE( refs1.size() == 3 );
  BOOST_REQUIRE( refs2.size() == 3 );

  int Ni1 = OBStereo::NumInversions(refs1);
  int Ni2 = OBStereo::NumInversions(refs2);

  return ((Ni1 + Ni2) % 2 == 0);
}
  
void mtest_configStruct()
{
  // reference Config
  OBTetrahedralStereo::Config reference(0, 1, OBStereo::MakeRefs(2, 3, 4), OBStereo::Clockwise, OBStereo::ViewFrom);

  // test copying
  OBTetrahedralStereo::Config referenceCopy = reference;
  BOOST_REQUIRE( reference == referenceCopy );
  BOOST_REQUIRE( referenceCopy == reference );

  // invalid center (chiral) id
  OBTetrahedralStereo::Config invalidCenter(45, 1, OBStereo::MakeRefs(2, 3, 4), OBStereo::Clockwise, OBStereo::ViewFrom);
  BOOST_REQUIRE( reference != invalidCenter );

  // invalid from/towards id
  OBTetrahedralStereo::Config invalidFrom(0, 45, OBStereo::MakeRefs(2, 3, 4), OBStereo::Clockwise, OBStereo::ViewFrom);
  BOOST_REQUIRE( reference != invalidFrom );

  // test other refs
  OBTetrahedralStereo::Config cfg1(0, 1, OBStereo::MakeRefs(2, 4, 3), OBStereo::Clockwise, OBStereo::ViewFrom);
  BOOST_REQUIRE( reference != cfg1 );

  // test anti-clockwise
  OBTetrahedralStereo::Config cfg2(0, 1, OBStereo::MakeRefs(2, 3, 4), OBStereo::AntiClockwise, OBStereo::ViewFrom);
  BOOST_REQUIRE( reference != cfg2 );

  // test viewing towards
  OBTetrahedralStereo::Config cfg3(0, 1, OBStereo::MakeRefs(2, 3, 4), OBStereo::Clockwise, OBStereo::ViewTowards);
  BOOST_REQUIRE( reference != cfg3 );

  // test anti-clockwise + viewing towards
  OBTetrahedralStereo::Config cfg4(0, 1, OBStereo::MakeRefs(2, 3, 4), OBStereo::AntiClockwise, OBStereo::ViewTowards);
  BOOST_REQUIRE( reference == cfg4 );

}

void mtest_IsValid()
{
  OBTetrahedralStereo ts(nullptr);
  OBTetrahedralStereo::Config cfg, cfgCopy;
  cfg.center = 0;
  cfg.from = 1;
  cfg.refs = OBStereo::MakeRefs(2, 3, 4);

  ts.SetConfig(cfg);
  BOOST_REQUIRE( ts.IsValid() );

  // no center
  cfgCopy = cfg;
  cfgCopy.center = OBStereo::NoRef;
  ts.SetConfig(cfgCopy);
  BOOST_REQUIRE( !ts.IsValid() );

  // no from
  cfgCopy = cfg;
  cfgCopy.from = OBStereo::NoRef;
  ts.SetConfig(cfgCopy);
  BOOST_REQUIRE( !ts.IsValid() );

  // no refs
  cfgCopy = cfg;
  cfgCopy.refs = std::vector<unsigned long>();
  ts.SetConfig(cfgCopy);
  BOOST_REQUIRE( !ts.IsValid() );
}

void mtest_equalsOperator()
{
  OBTetrahedralStereo ts1(nullptr), ts2(nullptr);
  OBTetrahedralStereo::Config cfg;
  cfg.center = 0;
  cfg.from = 1;
  cfg.refs = OBStereo::MakeRefs(2, 3, 4);

  ts1.SetConfig(cfg);
  ts2.SetConfig(cfg);
  BOOST_REQUIRE( ts1 == ts2 );

  cfg.winding = OBStereo::AntiClockwise;
  ts2.SetConfig(cfg);
  BOOST_REQUIRE( ts1 != ts2 );
}

void mtest_GetSetConfig()
{
  OBTetrahedralStereo th(nullptr);
  OBTetrahedralStereo::Config cfg;

  // set clockwise, viewing from 1
  BOOST_REQUIRE( !th.IsValid() );
  cfg.refs = OBStereo::MakeRefs(2, 3, 4);
  cfg.from = 1;
  cfg.center = 5;
  th.SetConfig(cfg);
  BOOST_REQUIRE( th.IsValid() );

  OBTetrahedralStereo::Config cfg2 = th.GetConfig();
  BOOST_REQUIRE( cfg2.center == 5 );
  BOOST_REQUIRE( cfg2.from == 1 );
  BOOST_REQUIRE( cfg2.refs.size() == 3 );
  BOOST_REQUIRE( cfg2.refs[0] == 2 );
  BOOST_REQUIRE( cfg2.refs[1] == 3 );
  BOOST_REQUIRE( cfg2.refs[2] == 4 );
  BOOST_REQUIRE( cfg2.winding == OBStereo::Clockwise );
  BOOST_REQUIRE( cfg2.view == OBStereo::ViewFrom );
  BOOST_REQUIRE( cfg == cfg2 );

  // get viewing from 3
  cfg2 = th.GetConfig(3);
  BOOST_REQUIRE( cfg2.from == 3 );
  BOOST_REQUIRE( cfg == cfg2 );

  // get viewing from 3, AC
  cfg2 = th.GetConfig(3, OBStereo::AntiClockwise);
  BOOST_REQUIRE( cfg2.winding == OBStereo::AntiClockwise );
  BOOST_REQUIRE( cfg == cfg2 );

  // get viewing towards 3
  cfg2 = th.GetConfig(3, OBStereo::Clockwise, OBStereo::ViewTowards);
  BOOST_REQUIRE( cfg2.view == OBStereo::ViewTowards );
  BOOST_REQUIRE( cfg == cfg2 );

  // get viewing towards 3, AC
  cfg2 = th.GetConfig(3, OBStereo::AntiClockwise, OBStereo::ViewTowards);
  BOOST_REQUIRE( cfg2.winding == OBStereo::AntiClockwise );
  BOOST_REQUIRE( cfg2.view == OBStereo::ViewTowards );
  BOOST_REQUIRE( cfg == cfg2 );
}

void test_Refs()
{
  OBTetrahedralStereo th(nullptr);
 
  // center 2, view from 1, clockwise 9 4 34
  OBTetrahedralStereo::Config cfg;
  cfg.center = 2;
  cfg.from = 1;
  cfg.refs = OBStereo::MakeRefs(9, 4, 34);
  th.SetConfig(cfg);

  BOOST_REQUIRE( th.IsValid() );

  // 
  // test viewing from/towards all atoms
  //

  // from/towards 1
  BOOST_REQUIRE( th.GetConfig() == cfg ); // from 1, clockwise
  cfg.winding = OBStereo::AntiClockwise;
  BOOST_REQUIRE( th.GetConfig() != cfg ); // from 1, anti-clockwise
  cfg.view = OBStereo::ViewTowards;
  BOOST_REQUIRE( th.GetConfig() == cfg ); // towards 1, anti-clockwise

  
  // from/towards 9
  OBTetrahedralStereo::Config cfg2;
  cfg2.center = 2;
  cfg2.from = 9;
  cfg2.refs = OBStereo::MakeRefs(4, 1, 34);
  BOOST_REQUIRE( th.GetConfig() == cfg2 ); // from 9, clockwise
  cfg2.winding = OBStereo::AntiClockwise;
  BOOST_REQUIRE( th.GetConfig() != cfg2 ); // from 9, anti-clockwise
  cfg2.view = OBStereo::ViewTowards;
  BOOST_REQUIRE( th.GetConfig() == cfg2 ); // towards 9, anti-clockwise

  // from/towards 4
  OBTetrahedralStereo::Config cfg3;
  cfg3.center = 2;
  cfg3.from = 4;
  cfg3.refs = OBStereo::MakeRefs(1, 9, 34);
  BOOST_REQUIRE( th.GetConfig() == cfg3 ); // from 4, clockwise
  cfg3.winding = OBStereo::AntiClockwise;
  BOOST_REQUIRE( th.GetConfig() != cfg3 ); // from 4, anti-clockwise
  cfg3.view = OBStereo::ViewTowards;
  BOOST_REQUIRE( th.GetConfig() == cfg3 ); // towards 4, anti-clockwise

  // from/towards 34
  OBTetrahedralStereo::Config cfg4;
  cfg4.center = 2;
  cfg4.from = 34;
  cfg4.refs = OBStereo::MakeRefs(4, 9, 1);
  BOOST_REQUIRE( th.GetConfig() == cfg4 ); // from 34, clockwise
  cfg4.winding = OBStereo::AntiClockwise;
  BOOST_REQUIRE( th.GetConfig() != cfg4 ); // from 34, anti-clockwise
  cfg4.view = OBStereo::ViewTowards;
  BOOST_REQUIRE( th.GetConfig() == cfg4 ); // towards 34, anti-clockwise

  //
  // test invalid Config structs
  //
  OBTetrahedralStereo::Config cfg5;
  BOOST_REQUIRE( th.GetConfig() != cfg5 );
  cfg5.center = 2;
  BOOST_REQUIRE( th.GetConfig() != cfg5 );
  cfg5.from = 34;
  BOOST_REQUIRE( th.GetConfig() != cfg5 );
  cfg5.refs = OBStereo::MakeRefs(4, 9, 1);
  BOOST_REQUIRE( th.GetConfig() == cfg5 );
  cfg5.from = OBStereo::NoRef;
  BOOST_REQUIRE( th.GetConfig() != cfg5 );
  cfg5.center = OBStereo::NoRef;
  cfg5.from = 34;
  BOOST_REQUIRE( th.GetConfig() != cfg5 );
}

BOOST_AUTO_TEST_CASE(tetrahedraltest)
{
  int defaultchoice = 1;
  
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  switch(choice) {
  case 1:
    mtest_configStruct();
    break;
  case 2:
    mtest_IsValid();
    break;
  case 3:
    mtest_equalsOperator();
    break;
  case 4:
    mtest_GetSetConfig();
    break;
  case 5:
    test_Refs();
    break;
  default:
    cout << "Test number " << choice << " does not exist!\n";
    return -1;
  }

  return 0;
}
