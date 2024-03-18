{
int t = 3;
int z = 4;
int zip = (t-1)*6 + z;

cout <<"The zip detector is " << zip << endl;

TFile f("../testOutput/170319_1616_RRQ_even.root");
TTree* zipTree = f.Get(Form("zip%d", zip));
TTree* eventTree = f.Get("EventTree");
zipTree->AddFriend(eventTree);

zipTree->Scan("EventNumber:PAOFchisq:PAOFdelay", "EventNumber>60000");
//zipTree->Scan("EventNumber:QIstd","EventNumber>20000");
//zipTree->Scan("EventNumber:QSOFdelay:QSOFchisq:QIOFvolts:QOOFvolts:QIOFvolts0:QOOFvolts0","EventNumber>20000 && EventNumber<60500");

}
