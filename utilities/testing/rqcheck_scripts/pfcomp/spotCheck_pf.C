{
int t = 5;
int z = 6;
int zip = (t-1)*6 + z;

cout <<"The zip detector is " << zip << endl;

TFile f(Form("../testOutput/test_170319_1616_F0006.root.t%dz%d", t, z));
TTree* zipTree = f.Get(Form("t%dz%d", t, z));

zipTree->Scan("zevent:pa.ped","zevent==60475");

}
