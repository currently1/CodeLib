//
#include <cSystem.h>
#include <cTradingPlatform.h>
#include <cTraderSpi.h>
#include <cTick.h>

#include <iostream>
#include <string>
#include "easylogging\easylogging++.h"

using std::string;


#ifndef _DEBUG
#define _DEBUG 0
#endif
bool CompareStringArray( const cArray< cString >& strArray1, const cArray< cString >& strArray2 )
{
	if( &strArray1 == &strArray2 )
		return true;
	if( strArray1.getSize() != strArray2.getSize() )
		return false;

	for( int i = 0; i < strArray1.getSize(); ++i )
	{
		cString str1 = strArray1[i];
		int j;
		for( j = 0; j < strArray2.getSize(); ++j )
		{
			
			if( Compare( str1, strArray2[j] ) )
				break;
		}
		if( j >= strArray2.getSize() )
			return false;
	}
	return true;
}

cTradingPlatform::cTradingPlatform()
: m_pTraderSpi( NULL )
, m_nRequestID( 0 )
, m_runAutoTrade( true )
{
	if( m_pMarketDataEngine.get() )
		m_pMarketDataEngine.reset();

	if( m_pStrategy.get() )
		m_pStrategy.reset();

	m_pPositions = make_shared< cPositionCollection >();
	m_pOrders = make_shared< cOrderCollection >();
	m_pTrades = make_shared< cTradeCollection >();
	m_pSubscribeInst = make_shared<vector<string>>();
	m_pInstMessageMap = new map<string, CThostFtdcInstrumentField*>();
	m_pInstCommissionRate = new map<string,shared_ptr< CThostFtdcInstrumentCommissionRateField>>();

//	m_pInstMessageMap = make_shared<map<string,CThostFtdcInstrumentField*>>();
}

cTradingPlatform::~cTradingPlatform()
{
	ClearPlatform();
}

void cTradingPlatform::RegisterTraderSpi( cTraderSpi* pTraderSpi )
{
	if( m_pTraderSpi == pTraderSpi )
		return;

	m_pTraderSpi = pTraderSpi;
	
	m_pTraderSpi->RegisterPositionCollection( m_pPositions );
	m_pTraderSpi->RegisterOrderCollection( m_pOrders );
	m_pTraderSpi->RegisterTradeCollection( m_pTrades );
	m_pTraderSpi->RegisterInstMessageMap(m_pInstMessageMap);
	m_pTraderSpi->RegisterInstCommissionMap(m_pInstCommissionRate);
	m_pTraderSpi->RegisterSubscribeInstList(m_pSubscribeInst);
	//cArray< cString > instrumentIDs;
	//pTraderSpi->GetInstrumentIDs( instrumentIDs );

	//if( !m_instrumentIDs.getSize() )
	//{		
	//	for( int i = 0; i < instrumentIDs.getSize(); ++i )
	//		m_instrumentIDs.push_back( instrumentIDs[i] );
	//}
	//else
	//{
	//	if( !CompareStringArray( m_instrumentIDs, instrumentIDs ) )
	//		yr_error( "cTraderSpi and cTradingPlatform has different instruments" );
	//}

	//for( int i = 0; i < m_instrumentIDs.getSize(); ++i )
	//{
	//	map< cString, double >::iterator it = m_closedPnL.find( m_instrumentIDs[i] );
	//	if( it == m_closedPnL.end() )
	//	{
	//		m_closedPnL.insert( map< cString, double >::value_type( m_instrumentIDs[i], 0.0 ) );
	//	}
	//}
}


void cTradingPlatform::RegisterMdSpi( cMdSpi* p )
{
	if( m_pMdSpi == p )
		return;

	m_pMdSpi = p;
}


void cTradingPlatform::RegisterMarketDataEngine( cMarketDataCollectionPtr pMarketDataEngine )
{
	if( m_pMarketDataEngine.get() )
		m_pMarketDataEngine.reset();

	m_pMarketDataEngine = pMarketDataEngine;
/*
	cArray< cString > instrumentIDs;
	pMarketDataEngine->GetInstrumentIDs( instrumentIDs );

	if( !m_instrumentIDs.getSize() )
	{	
		
		for( int i = 0; i < instrumentIDs.getSize(); ++i )
			m_instrumentIDs.push_back( instrumentIDs[i] );
	}
	else
	{
		if( !CompareStringArray( m_instrumentIDs, instrumentIDs ) )
			yr_error( "cMarketDataCollection and cTradingPlatform has different instruments" );
	}

	for( int i = 0; i < m_instrumentIDs.getSize(); ++i )
	{
		map< cString, double >::iterator it = m_closedPnL.find( m_instrumentIDs[i] );
		if( it == m_closedPnL.end() )
		{
			m_closedPnL.insert( map< cString, double >::value_type( m_instrumentIDs[i], 0.0 ) );
		}
	}
*/
}

void cTradingPlatform::RegisterStrategy( cStrategyPtr pStrategy )
{

}


void cTradingPlatform::RegisterParameters(autoSetting *p,mongoSetting *pM)
{
	this->m_pAutoSetting = p;
    this->m_pMongoSetting = pM;
    this->m_pMarketDataEngine->registerMongoSetting(pM);

}
void cTradingPlatform::SendNewOrder( cOrder* pOrder )
{
	
}

void cTradingPlatform::PrintClosePnL( int tradeID ) const
{


}


const sInstrumentInfo* cTradingPlatform::GetInstrumentInfo( const cString& instrumentID ) const
{
	if( m_pTraderSpi )
		return m_pTraderSpi->GetInstrumentInfo( instrumentID );
	else
	{
		return NULL;
		// demo paper trading
		//@todo
	}
}

const sTradingAccountInfo* cTradingPlatform::GetTradingAccountInfo() const
{
	if( m_pTraderSpi )
		return m_pTraderSpi->GetTradingAccountInfo();
	else
	{
		return NULL;
		// demo paper trading
		//@todo

	}
}

void cTradingPlatform::CancelPendingOrders()
{

}

void cTradingPlatform::CancelPendingOrders( const cString& instrumentID )
{

}

void cTradingPlatform::CancelPendingOrder( int orderID )
{
	//cOrder* pOrder = m_pOrders->GetOrderHandle( orderID );
	//if( m_pTraderSpi )
	//{
	//	if( pOrder && pOrder->IsPendingOrder() )
	//		m_pTraderSpi->ReqOrderAction( pOrder );
	//}
	//else
	//{
	//	//
	//	/*
	//		here we do the demo trade and no TraderSpi is required
	//		and thus we simply change m_orders
	//	*/
	//	m_pOrders->Remove( orderID );
	//}
}
//字符串分割函数  

std::vector<std::string> cTradingPlatform::splitToStr(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作  
    int size = str.size();

    for (int i = 0; i<size; i++)
    {
        pos = str.find(pattern, i);
        if (pos<size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}


std::vector<int32> cTradingPlatform::splitToInt(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<int32> result;
    str += pattern;//扩展字符串以方便操作  
    int size = str.size();

    for (int i = 0; i<size; i++)
    {
        pos = str.find(pattern, i);
        if (pos<size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(std::atoi(s.c_str()));
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
void cTradingPlatform::initStrategy(autoSetting & para){
    
    std::vector<std::string> instList = this->splitToStr(std::string(para.inst),",");
    std::vector<int32> lotsList = this->splitToInt(std::string(para.lots),",");
    std::vector<int32> timeModeList = this->splitToInt(std::string(para.timeMode),",");
    std::vector<std::string> collectionList = this->splitToStr(std::string(para.collectionList), ",");

    if (instList.size() != lotsList.size() || timeModeList.size() != lotsList.size()|| collectionList.size() != lotsList.size()){
        LOG(ERROR) << " initStrategy  inst lot timeMode Error, init Strategy Failed";
        return;
    }
    for (int i = 0;i < instList.size();i++) {
        std::shared_ptr<cStrategyKingKeltner> pStrategy = std::make_shared<cStrategyKingKeltner>();
        
        pStrategy->RegisterMarketDataCollection(this->m_pMarketDataEngine);
        pStrategy->RegisterTradeSpi(this->m_pTraderSpi);
        pStrategy->RegisterMdSpi(this->m_pMdSpi);
        pStrategy->RegisterPositionCollectionPtr(this->m_pPositions);
        pStrategy->RegisterOrderCollectionPtr(this->m_pOrders);
        pStrategy->RegisterTradeCollectionPtr(this->m_pTrades);
        pStrategy->RegisterTxtDir(string(para.tradeDayDir), string(para.dataBaseDir));
        pStrategy->RegisterAutoSetting(&para);


        this->readDay(string(para.tradeDayDir), this->m_tradeDayList);
        this->m_pMarketDataEngine->setTradeDayList(&this->m_tradeDayList);

        pStrategy->setInst(instList[i]);
        pStrategy->setlots(lotsList[i]);
        pStrategy->setTimeMode(timeModeList[i]);
        pStrategy->setInitDate(para.startDate, para.endDate);
        pStrategy->setCollectionName(collectionList[i]);

        this->m_pTraderSpi->RegisterStrategy(pStrategy.get());
        this->m_StrategyKKList.push_back(pStrategy);
    }





}

void cTradingPlatform::readDay(string fileName, map<string,int> &workDay){
	ifstream file1(fileName,ios::in);	//以只读方式读入,读取原始数据
	char dataline[512];//行数变量
	string date;
	int i = 1;
	if(!file1){
		cout<<"no such file!"<<endl;
		//abort();
		return;
	}	
	while(file1.getline(dataline,1024,'\n'))//while开始，读取一行1024够大？
	{
		//sscanf_s(dataline,"%s",date);
		date = dataline;
		workDay.insert(pair<string,int>(date,i));
		i++;
		//cout << date << endl;

	}
}

DWORD cTradingPlatform::AutoTrading()
{

	string str;
	char dire[50],offset[50],inst[50],price[50],order[50],tag[50];
	int vol, mark = 0,orderNo;
	cerr<<"--------------------Human-computer interaction function Start--------------------------------"<<endl;
	cerr<<endl<<"OrderList: help | show | order| trade | stop | run |close |buy/sell open/close inst vol price| cancle seqNo：";
	//initial subcribe instrument
	this->m_pMdSpi->SubscribeMarketData(this->m_pSubscribeInst);
	this->initStrategy(*(this->m_pAutoSetting));
	this->m_pTraderSpi->RegisterMarketDataEngine(this->m_pMarketDataEngine);
	while(true)
	{
		//std::cin>>str;
		memset(price,0,50);
		memset(order,0,50);
		memset(tag,0,50);
		vol = 0;
		orderNo = 0;
		getline(std::cin,str);
		if(str == "show"){
			m_pTraderSpi->showPositionDetail();
		}

		else if(str == "close")
		{	
			cerr<<"close Position:"<<endl;
			//g_pUserSpi_tradeAll->ForceClose();
		}
		else if(str == "run")
		{			
//			m_strategy.start();
            for (auto iter : m_StrategyKKList) {
                iter->start();
            }
					}
		else if(str == "stop")
		{
		//	m_strategy.stop();
            for (auto iter : m_StrategyKKList) {
                iter->stop();
            }
           
		}
		else if(str == "order"){
			this->m_pOrders->PrintPendingOrders();
		}
		else if(str == "trade"){
			// 首先查询手续费 再查询成交
			this->m_pTraderSpi->ReqQryInstrumentCommissionRate(true);
		}else if(str == "help"){
			cerr<<"OrderList: show | order| trade | stop | run |close |buy/sell open/close inst vol price -> ";
		}
		else if(str == "account"){
			this->m_pTraderSpi->ReqQryTradingAccount();
		}
		else if(str.length() >7)
		{
			// insert order
			sscanf(str.c_str(),"%s %s %s %d %s",dire,offset,inst,&vol,price);
			double dPrice = atof(price);
			if(vol!=0){
				this->insertOrder(string(inst),string(dire),string(offset),vol,dPrice);
			}
			// cancle order
			sscanf(str.c_str(),"%s %d",order,&orderNo);
			if(orderNo !=0 ){
				this->cancleOrder(order,orderNo);
			}
			

		}
	}
	return 0;
}


DWORD cTradingPlatform::ProcessOrderTest()
{
	return 0;
}

DWORD cTradingPlatform::IOProcess()
{
//	return m_pMarketDataEngine->IOProcess();
	return 0;
}

DWORD cTradingPlatform::SimulationIOProcess()
{
	//if( m_pMarketDataEngine.get() )
	//	return m_pMarketDataEngine->SimulationIOProcess();
	//else
	//	return 0;
	return 0;
}


void cTradingPlatform::PrintPendingOrders() const
{
	m_pOrders->PrintPendingOrders();
}

void cTradingPlatform::PrintCancelledOrders() const
{
	m_pOrders->PrintCancelledOrders();
}

void cTradingPlatform::PrintAllOrders() const
{
	m_pOrders->PrintAllOrders();
}

void cTradingPlatform::PrintPositionSummary() const
{
//	m_pPositions->SummaryByInstrument();
}



bool cTradingPlatform::SimulationUpdate( const cTick& tick )
{
	//if( m_pTraderSpi )
	//	return false;

	//m_pMarketDataEngine->SimulationUpdate( tick );
	//bool flag = m_pStrategy->SimTimeIndicatorSignalUpdate( tick );
	//return flag;
	return true;
}

void cTradingPlatform::Sleep()
{
	int hh = cSystem::HourNow();
	int mm = cSystem::MinuteNow();
	bool flag = false;
	if( hh > 2 && hh < 8 )
		flag = true;
	else if( hh == 8 && mm < 30 )
		flag = true;

	if( flag )
	{
		//
		if( m_pTrades->Count() )
			m_pTrades->Clear();
		//
		if( m_pOrders->Count() )
			m_pOrders->Clear();

	}
}

void cTradingPlatform::WakeUp()
{

}


void cTradingPlatform::ClearPlatform()
{
//	/*m_instrumentIDs.clear();*/
	m_runAutoTrade = false;
	m_nRequestID = 0;
	m_pTraderSpi = NULL;

	/*if( m_pMarketDataEngine.get() )
		m_pMarketDataEngine.reset();

	if( m_pStrategy.get() )
		m_pStrategy.reset();*/

	if( m_pPositions.get() )
	{
//		m_pPositions->Clear();
		m_pPositions.reset();
	}

	if( m_pOrders.get() )
	{
		m_pOrders->Clear();
		m_pOrders.reset();
	}

	if( m_pTrades.get() )
	{
		m_pTrades->Clear();
		m_pTrades.reset();
	}
	// 
	delete this->m_pInstCommissionRate;
	delete this->m_pInstMessageMap;

}
void cTradingPlatform::cancleOrder(string order,int seqNo){
	if(order == "cancle"){
		
		shared_ptr<cOrder> pOrder = NULL;
		if(seqNo != -1){
			if(!this->m_pOrders->getOrderByNo(seqNo,pOrder)){
				cerr<<"  Order Seq No Exist."<<endl;
				return;
			}
			this->m_pTraderSpi->ReqOrderAction(pOrder);
		}else{
			this->m_pTraderSpi->cancleAllPendingOrder();
		}
	}
}
void cTradingPlatform::cancleAllOrder(string order,string tag){
	//if(order == "cancle" && tag == "all"){
	//	vector<cOrderPtr> vOrder = this->m_pOrders->GetAllOrder();
	//	for(auto it = vOrder.begin();it!=vOrder.end();it++){

	//		this->m_pTraderSpi->ReqOrderAction(it->get()->GetOrderID());
	//	}
	//}
}

void cTradingPlatform::insertOrder(string inst,string dire,string flag, int vol,double orderPrice){
	// get parameters type
	DIRECTION eDire;
	OFFSETFLAG eFlag;
	if(dire == "buy"){
		eDire = DIRECTION::buy;
	}
	else if(dire == "sell"){
		eDire =	DIRECTION::sell;
	}else{
		cerr << "input parameter Error" << endl;
		return;
	}
	
	if(flag == "open"){
		eFlag = OFFSETFLAG::open;
	}
	else if(flag == "close"){
		eFlag = OFFSETFLAG::close;;
	}else{
		cerr << "input parameter Error" << endl;
		return;
	}

	
	// go into order
	this->m_pTraderSpi->insertOrder(inst,eDire,eFlag,vol,orderPrice);
	_sleep(500);// wait 500ms for pTrader response.
}