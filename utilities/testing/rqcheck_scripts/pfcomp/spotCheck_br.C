{
int t = 5;
int z = 3;
int zip = (t-1)*6 + z;

cout <<"The zip detector is " << zip << endl;

TFile f("../testOutput/test_170319_1616_F0006.gz.root");
TTree* zipTree = f.Get(Form("zip%d", zip));
TTree* eventTree = f.Get("eventTree");
zipTree->AddFriend(eventTree);

zipTree->Scan("EventNumber:PAbs");

}
