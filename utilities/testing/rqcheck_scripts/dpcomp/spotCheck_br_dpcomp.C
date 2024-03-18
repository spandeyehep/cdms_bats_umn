{
int t = 3;
int z = 4;
int zip = (t-1)*6 + z;

cout <<"The zip detector is " << zip << endl;

TFile f("../testOutput/test_170319_1616_F0006.root");
TTree* zipTree = f.Get(Form("zip%d", zip));
TTree* eventTree = f.Get("eventTree");
zipTree->AddFriend(eventTree);

//zipTree->Scan("EventNumber:(QIrms*0.8e-6)","(EventNumber%2)==0");
//zipTree->Scan("EventNumber:QSOFdelay:QSOFchisq:QIOFvolts:QOOFvolts:QIOFvolts0:QOOFvolts0","(EventNumber%2)==0");
zipTree->Scan("EventNumber:PAOFchisq:PAOFdelay","(EventNumber%2)==0");

}
