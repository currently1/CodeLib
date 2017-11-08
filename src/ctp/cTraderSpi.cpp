#include <cSystem.h>
#include <cTraderSpi.h>

#include <cTrade.h>
#define ROHON 1
#undef  ROHON
#define _CTP 1

extern HANDLE g_hEvent;
bool IsFlowControl( int iResult )
{
	return (( iResult == -2 ) || ( iResult == -3 ));
}

cTraderSpi::cTraderSpi( CThostFtdcTraderApi* pUserTraderApi,cMdSpi* pUserMdSpi,CThostFtdcMdApi * pUserMdApi,TThostFtdcBrokerIDType brokerID, TThostFtdcInvestorIDType investorID, char* password, bool genLog )
: m_pUserTraderApi( pUserTraderApi )
, m_genLog( genLog )
{
	strcpy_s( m_brokerID, sizeof( TThostFtdcBrokerIDType ), brokerID );
	strcpy_s( m_investorID, sizeof( TThostFtdcInvestorIDType ), investorID );
	strcpy_s( m_password, sizeof( m_password), password );
	this->m_pMdSpi = pUserMdSpi;
	this->m_pMDUserApi_td = pUserMdApi;
	m_first_inquiry_order = true;//�Ƿ��״β�ѯ����
	m_first_inquiry_trade = true;//�Ƿ��״β�ѯ�ɽ�
	m_firs_inquiry_Detail = true;//�Ƿ��״β�ѯ�ֲ���ϸ
	m_firs_inquiry_TradingAccount = true;//�Ƿ��״β�ѯ�ʽ��˺�
	m_firs_inquiry_Position = true;//�Ƿ��״β�ѯͶ���ֲ߳�
	m_first_inquiry_Instrument = true;//�Ƿ��״β�ѯ��Լ
	m_first_inquiry_commissionRate = true;// �Ƿ��״β�ѯ
	m_closeProfit = 0.0;//ƽ��ӯ��
	m_OpenProfit = 0.0;//����ӯ��

	m_accountMargin = 0.0;

	m_InstMeassageMap = NULL;
}

cTraderSpi::~cTraderSpi()
{
	delete m_accountInfo;
	
}

// After making a succeed connection with the CTP server, the client should send the login request to the CTP server.
void cTraderSpi::OnFrontConnected()
{
	char message[256];
	sprintf( message, "%s:called cTraderSpi::OnFrontConnected.", cSystem::GetCurrentTimeBuffer().c_str() );
	cout << message << endl;


	ReqUserLogin();
	SetEvent(g_hEvent);
}

void cTraderSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	
	memset( &req, 0, sizeof( req ) );
	strcpy_s( req.BrokerID, sizeof( TThostFtdcBrokerIDType ), m_brokerID );
	strcpy_s( req.UserID, sizeof( TThostFtdcInvestorIDType ), m_investorID );
	strcpy( req.Password, m_password );

	#ifdef _CTP

		int iResult = m_pUserTraderApi->ReqUserLogin( &req, ++iRequestID );
		char message[256];
		sprintf( message, "%s:called cTraderSpi::ReqUserLogin: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
		cout << message << endl;	

	#else
	//����һ������� ������debugģʽ��ʹ�øó���release��Ϊ��ʹ�ù�˾�ļ��ܽ���dll ע������ѡ�� �޳����Ż� - ���ֽ� ͬʱ Ԥ����Ϊ��WIN32;_SCL_SECURE_NO_WARNINGS
		HINSTANCE hInst = LoadLibrary(TEXT("dll_FBI_Release_Win32.dll"));
		DWORD errorId = GetLastError();
		ccbf_secureApi_LoginTrader ccbf_traderFuncInterface = (ccbf_secureApi_LoginTrader)GetProcAddress(hInst,"ccbf_secureApi_LoginTrader_After_CTP_OnConnected");
		if(!ccbf_traderFuncInterface)
		{
			cerr <<"Interface func error"<<endl;
		}else{
			int ret = ccbf_traderFuncInterface(m_pUserTraderApi, req.BrokerID, req.UserID, req.Password, iRequestID);
			//int ret = m_pUserApi_td->ReqUserLogin(&req, ++requestId);
			cerr<<"Trader login request..."<<((ret == 0) ? "success" :"fail") << endl;	
			FreeLibrary(hInst);
		}
	#endif
	SetEvent(g_hEvent);
}

// When the connection between client and the CTP server disconnected, the following function will be called
void cTraderSpi::OnFrontDisconnected(int nReason)
{
	// in this case, API will reconnect, the client application can ignore this
	char message[256];
	sprintf( message, "%s:called cTraderSpi::OnFrontDisconnected. Reason = %d", cSystem::GetCurrentTimeBuffer().c_str(), nReason );
	cout << message << endl;
	
}

// After receiving the login request from the client, the CTP server will send the following response to notify the client whether the login success or not
void cTraderSpi::OnRspUserLogin( CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	if( !IsErrorRspInfo( pRspInfo )&&pRspUserLogin )
	{
		m_FRONT_ID = pRspUserLogin->FrontID;
		m_SESSION_ID = pRspUserLogin->SessionID;
		int iNextOrderRef = atoi( pRspUserLogin->MaxOrderRef );
		m_tradeDay = string(pRspUserLogin->TradingDay);
		m_actionDay = cSystem::GetCurrentDayBuffer();
		iNextOrderRef++;
		sprintf( m_ORDER_REF, "%d", iNextOrderRef );


		char message[256];
		sprintf( message, "%s:called cTraderSpi::OnRspUserLogin.", cSystem::GetCurrentTimeBuffer().c_str() );
		cout << message << endl;

		ReqSettlementInfoConfirm();
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void cTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset( &req, 0, sizeof( req ) );
	strcpy( req.BrokerID, m_brokerID );
	strcpy( req.InvestorID, m_investorID );
	int iResult = m_pUserTraderApi->ReqSettlementInfoConfirm( &req, ++iRequestID );


	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqSettlementInfoConfirm: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
	cout << message << endl;
}



void cTraderSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	if(!IsErrorRspInfo( pRspInfo ) && pSettlementInfoConfirm )
	{
		char message[256];
		sprintf( message, "%s:called cTraderSpi::OnRspSettlementInfoConfirm.Success ", cSystem::GetCurrentTimeBuffer().c_str() );
		cout << message << endl;
		Sleep(1000);// wait CTP have enough time to response
		//ReqQryInstrument();
		ReqQryOrder();
	}
	if(bIsLast)SetEvent(g_hEvent);
}


void cTraderSpi::ReqQryOrder()
{
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.InvestorID, this->m_investorID);//Ͷ���ߴ���

	int iResult = m_pUserTraderApi->ReqQryOrder(&req, ++iRequestID);

	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryOrder: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
	cout << message << endl;

}


void cTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	

	if (!IsErrorRspInfo(pRspInfo) && pOrder )
	{
		if(m_first_inquiry_order == true)
		{
			this->m_orderCollection->Add(pOrder);
			
			if(bIsLast) 
			{
				m_first_inquiry_order = false;
				char message[256];
				sprintf( message, "%s:called cTraderSpi::OnRspQryOrder success.", cSystem::GetCurrentTimeBuffer().c_str());
				cout << message << endl;

				cerr<<"--------------------------------------------------------------------order start"<<endl;
				this->m_orderCollection->PrintAllOrders();
				cerr<<"--------------------------------------------------------------------order end"<<endl;

				Sleep(1000);
				cerr<<"First Qry Order Success"<<endl;
				ReqQryInvestorPositionDetail();

				SetEvent(g_hEvent);

			}
		}


	}
	else
	{
		if(m_first_inquiry_order == true ) 
		{
			m_first_inquiry_order = false;
			Sleep(1000);
			cerr<<"Error or no order"<<endl;
			ReqQryInvestorPositionDetail();
		}

	}


}


//request Query Investor posistion Detail
void cTraderSpi::ReqQryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.InvestorID, m_investorID);//investor Id

	int iResult = m_pUserTraderApi->ReqQryInvestorPositionDetail(&req, ++iRequestID);

	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryInvestorPositionDetail: %s.",  cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
	cout << message << endl;
}

///Request Query Investor position Detail,First Query & followed Query
void cTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr<<"OnRspQryInvestorPositionDetail"<<endl;

	if(!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail )
	{

		//all Instrument
		if(m_firs_inquiry_Detail == true)
		{	
			//�������к�Լ����������ƽ�ֵģ�ֻ����δƽ�ֵ�
			//Ϊʲô���ڲ�ѯ�ɽ��ص�����OnRspQryTrade()������,��Ϊֻ�ܲ�ѯ�����
			//ʹ�ýṹ����CThostFtdcTradeField����Ϊ����ʱ���ֶΣ���CThostFtdcInvestorPositionDetailFieldû��ʱ���ֶ�

			CThostFtdcTradeField* trade = new CThostFtdcTradeField();

			strcpy_s(trade->InvestorID,sizeof(TThostFtdcInvestorIDType), pInvestorPositionDetail->InvestorID);///InvestorID
			strcpy_s(trade->InstrumentID,sizeof(TThostFtdcInstrumentIDType), pInvestorPositionDetail->InstrumentID);///InstrumentID
			strcpy_s(trade->ExchangeID,sizeof(TThostFtdcExchangeIDType), pInvestorPositionDetail->ExchangeID);///ExchangeID
			trade->Direction = pInvestorPositionDetail->Direction;///Direction
			trade->Price = pInvestorPositionDetail->OpenPrice;///OpenPrice
			trade->Volume = pInvestorPositionDetail->Volume;///Volume
			strcpy_s(trade->TradeDate, sizeof(TThostFtdcDateType),pInvestorPositionDetail->OpenDate);///OpenDate


			if(pInvestorPositionDetail->Volume > 0)//ɸѡδƽ�ֵ�
			{
				m_tradeListNotClosedAccount.push_back(trade);
			}


			//������к�Լ�ĳֲ���ϸ��Ҫ����߽�����һ���Ĳ�ѯReqQryTradingAccount();
			if(bIsLast)
			{
				char message[256];
				sprintf( message, "%s:called cTraderSpi::OnRspQryInvestorPositionDetail success.", cSystem::GetCurrentTimeBuffer().c_str());
				cout << message << endl;

				cerr<<"All order��" << m_orderList.size()<<endl;
				m_firs_inquiry_Detail = false;

				cerr<<"----------------------------------------------Position Detail start"<<endl;

				for(vector<CThostFtdcTradeField*>::iterator iter = m_tradeListNotClosedAccount.begin(); iter != m_tradeListNotClosedAccount.end(); iter++)
				{
					cerr<<"InvestorID:"<<(*iter)->InvestorID
						<<" Inst:"<<(*iter)->InstrumentID
						<<" Exchange:"<<(*iter)->ExchangeID
						<<" Dire:"<<((*iter)->Direction == '0'? "long" : "short")
						<<" Price:"<<(*iter)->Price
						<<" vol:"<<(*iter)->Volume
						<<" tradeDate:"<<(*iter)->TradeDate
						<<" tradeTime:"<<(*iter)->TradeTime<<endl;
				}

				cerr<<"----------------------------------------------Position Detail end"<<endl;


				Sleep(1000);
				ReqQryTradingAccount();

			}
		}



	}
	else
	{
		if(m_firs_inquiry_Detail == true)
		{
			m_firs_inquiry_Detail = false;
			Sleep(1000);
		
			ReqQryTradingAccount();
		}

	}

	//cerr<<"-----------------------------------------------OnRspQryInvestorPositionDetail End"<<endl;

}

void cTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset( &req, 0, sizeof( req ) );
	strcpy( req.BrokerID, m_brokerID );
	strcpy( req.InvestorID, m_investorID );
	int iResult = m_pUserTraderApi->ReqQryTradingAccount( &req, ++iRequestID );
	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryTradingAccount: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
	cout << message << endl;
	
}

void cTraderSpi::OnRspQryTradingAccount( CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	//cerr<<"OnRspQryTradingAccount"<<endl;

	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount)
	{

		m_accountInfo = new sTradingAccountInfo;
		memset( m_accountInfo, 0, sizeof( sTradingAccountInfo ) );
		strcpy( m_accountInfo->BrokerID, pTradingAccount->BrokerID );
		strcpy( m_accountInfo->AccountID, pTradingAccount->AccountID );
		m_accountInfo->PreBalance = pTradingAccount->PreBalance;
		m_accountInfo->Balance = pTradingAccount->Balance;
		m_accountInfo->Available = pTradingAccount->Available;
		m_accountInfo->WithdrawQuota = pTradingAccount->WithdrawQuota;
		m_accountInfo->CurrMargin = pTradingAccount->CurrMargin;
		m_accountInfo->CloseProfit = pTradingAccount->CloseProfit;
		m_accountInfo->PositionProfit = pTradingAccount->PositionProfit;
		m_accountInfo->Commission = pTradingAccount->Commission;
		m_accountInfo->FrozenMargin = pTradingAccount->FrozenMargin;

		printf("\nAccount Summary:\n");
		printf("   AccountID:%s\n", m_accountInfo->AccountID );
		printf("   PreBalance:%.2f\n", m_accountInfo->PreBalance );
		printf("   Balance:%.2f\n", m_accountInfo->Balance );
		
		//printf("   WithdrawQuota:%f\n", m_accountInfo->WithdrawQuota );
		printf("   totalPnl:%.2f\n", m_accountInfo->CloseProfit +  m_accountInfo->PositionProfit);
		printf("   CloseProfit:%.2f\n", m_accountInfo->CloseProfit );
		printf("   PositionProfit:%.2f\n", m_accountInfo->PositionProfit );
		printf("   Commission:%.2f\n", m_accountInfo->Commission );
		printf("   Available:%.2f\n", m_accountInfo->Available );
		printf("   CurrMargin:%.2f\n", m_accountInfo->CurrMargin );
		//printf("   FrozenMargin:%f\n", m_accountInfo->FrozenMargin );

		if(m_firs_inquiry_TradingAccount == true)
		{
			m_firs_inquiry_TradingAccount = false;
			Sleep(1000);

			cerr<<"OnRspQryTradingAccount success"<<endl;		
			
			ReqQryInvestorPosition_all();
		}


	}

	else
	{
		if(m_firs_inquiry_TradingAccount == true)
		{
			m_firs_inquiry_TradingAccount = false;
			Sleep(1000);
			
			cerr<<"OnRspQryTradingAccount error"<<endl;			
			
			ReqQryInvestorPosition_all();
			
		}

	}
	if(bIsLast) SetEvent(g_hEvent);

//	cerr<<"-----------------------------------------------OnRspQryTradingAccount End"<<endl;
}
///
void cTraderSpi::ReqQryInvestorPosition_all()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	//strcpy(req.BrokerID, appId);
	//strcpy(req.InvestorID, userId);
	//strcpy(req.InstrumentID, instId);

	int iResult = m_pUserTraderApi->ReqQryInvestorPosition(&req, ++iRequestID);
	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryInvestorPosition: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail" ) );
	cout << message << endl;
}

//�ֲֲ�ѯ�ص�����,�Ѿ�ƽ�ֵĵ��ӣ��ֲ���Ϊ0�˻��᷵��
void cTraderSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{;

	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition)
	{
		if(m_firs_inquiry_Position == true)
		{
			//update position
			this->m_positionCollection->update(pInvestorPosition);
			// subscribe Instrument
			this->subscribeInst(pInvestorPosition->InstrumentID,false);

			if (bIsLast)
			{
				m_firs_inquiry_Position = false;

				this->m_positionCollection->PrintDetail();

				Sleep(1000);
				ReqQryInstrument_all();

			}

		}



	}
	else
	{
		if(m_firs_inquiry_Position == true)
		{
			m_firs_inquiry_Position = false;

			cerr<<"OnRspQryInvestorPosition Error or No position"<<endl;
			Sleep(1000);
			ReqQryInstrument_all();
		}

	}
}



void cTraderSpi::ReqQryInstrument_all()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));

	int iResult = m_pUserTraderApi->ReqQryInstrument(&req, ++iRequestID);
	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryInstrument: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail" ) );
	cout << message << endl;
}

void cTraderSpi::OnRspQryInstrument( CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	
	if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument)
	{
		if(m_first_inquiry_Instrument == true)
		{
			//save all instrument message map
			CThostFtdcInstrumentField* instField = new CThostFtdcInstrumentField();
			memcpy(instField,pInstrument, sizeof(CThostFtdcInstrumentField));
			m_InstMeassageMap->insert(pair<string, CThostFtdcInstrumentField*> (instField->InstrumentID, instField));

			//saveInstrumentField(pInstrument);

			if(bIsLast)
			{

				m_first_inquiry_Instrument = false;
				Sleep(1000);
				SetEvent(g_hEvent);
				//�ں���Ҫָ����Լ
				#ifdef  ROHON
					m_itMap = m_InstMeassageMap->begin();

					//First Start up
					m_output.open("output/" + string(m_investorID)  + "_" + m_tradeDay + "_commission.txt", ios::_Nocreate | ios::in) ;
					if(m_output){
						while(!m_output.eof()){
							//get Instrument List
							shared_ptr<CThostFtdcInstrumentCommissionRateField> instField = make_shared<CThostFtdcInstrumentCommissionRateField>();
							m_output >> instField->InstrumentID  >>
								instField->CloseRatioByMoney		>>instField->CloseRatioByVolume>>
								instField->CloseTodayRatioByMoney	>>instField->CloseTodayRatioByVolume>>
								instField->OpenRatioByMoney			>>instField->OpenRatioByVolume ;
							m_pInstCommissionMap->insert(pair<string, shared_ptr<CThostFtdcInstrumentCommissionRateField> > (instField->InstrumentID, instField));
						}

						if(m_pInstCommissionMap->size()>10){
							ReqQryTrade();
							return;
						}
					
					}
					m_output.open("output/" + string(m_investorID)   + "_" + m_tradeDay + "_commission.txt", ios::app|ios::out) ;

					ReqQryInstrumentCommissionRate();
				#else
					ReqQryInstrumentCommissionRate();
				#endif //  ROHON


			}

		}



	}

}
// ����ѭ����ѯ �����ѵķ�ʽ����Ϊ�ӿ�һ��ֻ�ܲ�ѯһ����Ϊ�˷��㣬��ѯ�����Ľ�� ���浽txt�У��ڶ�������ʱֱ�Ӵ��ļ���ȡ���ٶȾͿ�ܶ࣬���ǵ�һ�β�ѯ��Ҫ����10Min
void cTraderSpi::ReqQryInstrumentCommissionRate(bool qryTrade ){
	#ifdef ROHON
		if(qryTrade == true){
			ReqQryTrade();
			return;
		}
		CThostFtdcQryInstrumentCommissionRateField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.InvestorID, m_investorID);//investor Id
		strcpy(req.BrokerID, m_brokerID);//broker Id

		//m_pInstCommissionMap->clear();
		//cout << m_itMap->second->ExpireDate << " " << m_actionDay << (string(m_itMap->second->ExpireDate) == m_actionDay) << " " << string(m_itMap->second->InstrumentID).size() << endl;
		//system("pause");

		// Ϊ�˹�����Ϻ�Լ ���� XX-SR801&SR803 ���û��Ҫ��ѯ�������ˡ� ������Ϊ���յĺ�Լ ��ѯ��error������
		string instName;
		while (m_itMap != this->m_InstMeassageMap->end() &&
			(string(m_itMap->second->ExpireDate) == m_actionDay ||
			(!isValidInsturment(string(m_itMap->second->InstrumentID),instName)))){
				cerr << "ignore: " << m_itMap->second->InstrumentID << endl;
				m_itMap++;
		}
		// over QryTrade��
		if (m_itMap == this->m_InstMeassageMap->end()){
			ReqQryTrade();
		}
		strcpy(req.InstrumentID, m_itMap->second->InstrumentID);
		int iResult = m_pUserTraderApi->ReqQryInstrumentCommissionRate(&req, ++iRequestID);
		cerr << cSystem::GetCurrentTimeBuffer() <<" Qry:" << req.InstrumentID << (( iResult == 0 ) ? "Success" : "Fail") << endl;
	#else
		CThostFtdcQryInstrumentCommissionRateField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.InvestorID, m_investorID);//investor Id
		strcpy(req.BrokerID, m_brokerID);//broker Id
		m_pInstCommissionMap->clear();
		int iResult = m_pUserTraderApi->ReqQryInstrumentCommissionRate(&req, ++iRequestID);

		if(m_first_inquiry_commissionRate){
			m_first_inquiry_commissionRate = false;
			cerr << cSystem::GetCurrentTimeBuffer() <<" Qry:" << req.InstrumentID << (( iResult == 0 ) ? "Success" : "Fail") << endl;
		}
	#endif // ROHON



}

void cTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
#ifdef  ROHON
	if(!IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//save all instrument message map
		shared_ptr<CThostFtdcInstrumentCommissionRateField> instField = make_shared<CThostFtdcInstrumentCommissionRateField>(*pInstrumentCommissionRate);
		m_pInstCommissionMap->insert(pair<string, shared_ptr<CThostFtdcInstrumentCommissionRateField> > (pInstrumentCommissionRate->InstrumentID, instField));
		m_output << m_itMap->second->InstrumentID<<" "  <<
			instField->CloseRatioByMoney<<" "		<<instField->CloseRatioByVolume<<" "<<
			instField->CloseTodayRatioByMoney<<" "	<<instField->CloseTodayRatioByVolume<<" "<<
			instField->OpenRatioByMoney<<" "			<<instField->OpenRatioByVolume << endl;
		if(bIsLast){
			Sleep(1000);
			SetEvent(g_hEvent);
			m_itMap++;
			if(m_itMap != this->m_InstMeassageMap->end()){
				ReqQryInstrumentCommissionRate();
			}
			else{
				ReqQryTrade();
			}
		}
	}
	else
	{

		Sleep(1000);
		SetEvent(g_hEvent);
		cerr << "OnRspQryInstrumentCommissionRate::error" << endl;
		m_itMap++;
		if(m_itMap != this->m_InstMeassageMap->end()){
			ReqQryInstrumentCommissionRate();
		}
		else{
			ReqQryTrade();
		}


	}
#else
	if(!IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//save all instrument message map
		shared_ptr<CThostFtdcInstrumentCommissionRateField> instField = make_shared<CThostFtdcInstrumentCommissionRateField>(*pInstrumentCommissionRate);
		m_pInstCommissionMap->insert(pair<string, shared_ptr<CThostFtdcInstrumentCommissionRateField> > (pInstrumentCommissionRate->InstrumentID, instField));
		if(bIsLast){
			Sleep(1000);
			SetEvent(g_hEvent);
			ReqQryTrade();
		}
	}
	else
	{
		Sleep(1000);
		SetEvent(g_hEvent);
		cerr << "OnRspQryInstrumentCommissionRate::error" << endl;
		ReqQryTrade();
	}
#endif //  ROHON


}

void cTraderSpi::ReqQryTrade(){
	m_output.close();
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.InvestorID, this->m_investorID);//

	int iResult = m_pUserTraderApi->ReqQryTrade(&req, ++iRequestID);
	
	this->m_tradeCollection->Clear();

	char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqQryTrade: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail") );
	cout << message << endl;
}

void cTraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pTrade && strlen(pTrade->InvestorID) != 0){

		auto iter = (this->m_pInstCommissionMap->find(pTrade->InstrumentID) == this->m_pInstCommissionMap->end()? NULL : this->m_pInstCommissionMap->at(pTrade->InstrumentID));
		if(iter == NULL){
			string instName;
			this->isValidInsturment(pTrade->InstrumentID,instName);
			iter = (this->m_pInstCommissionMap->find(instName) == this->m_pInstCommissionMap->end()? NULL : this->m_pInstCommissionMap->at(instName));
		}
		this->m_tradeCollection->Add(pTrade,iter.get(),this->m_InstMeassageMap->at(pTrade->InstrumentID));

		if(bIsLast){
			cerr<<"--------------------------------------------------------------------Trade list start"<<endl;
			this->m_tradeCollection->PrintAll();
			cerr<<"--------------------------------------------------------------------Trade list end"<<endl;
			if(m_first_inquiry_trade){
				m_first_inquiry_trade = false;
				Sleep(500);
				cout<<"Trade Init Finish, start up MD:"<<endl;
				m_pMDUserApi_td->Init();
			}

			SetEvent(g_hEvent);
		}
	}else{
		cerr<<"Error or no trade"<<endl;
		if(m_first_inquiry_trade){
			m_first_inquiry_trade = false;
			Sleep(500);
			cerr<<"Trade Init Finish, start up MD:"<<endl;
			m_pMDUserApi_td->Init();
		}
		SetEvent(g_hEvent);
	}

}

void cTraderSpi::saveInstrumentField(CThostFtdcInstrumentField* instField){
	// ���� ���Ǳ��浽���ݿ���
	ofstream output;
	output.open("output/instrumentListField.csv",ios::_Nocreate | ios::ate) ;
	if(output){
		output  << instField->InstrumentID << "," 
				<< instField->ExchangeID << "," 
				<< instField->InstrumentName << "," 
				<< instField->ExchangeInstID << "," 
				<< instField->ProductID << "," 
				<< instField->ProductClass << "," 
				<< instField->DeliveryYear << "," 
				<< instField->DeliveryMonth << "," 
				<< instField->MaxMarketOrderVolume << "," 
				<< instField->MinMarketOrderVolume << "," 
				<< instField->MaxLimitOrderVolume << "," 
				<< instField->MinLimitOrderVolume << "," 
				<< instField->VolumeMultiple << "," 
				<< instField->PriceTick << "," 
				<< instField->CreateDate << "," 
				<< instField->OpenDate << "," 
				<< instField->ExpireDate << "," 
				<< instField->StartDelivDate << "," 
				<< instField->EndDelivDate << "," 
				<< instField->InstLifePhase << "," 
				<< instField->IsTrading << "," 
				<< instField->PositionType << "," 
				<< instField->PositionDateType << "," 
				<< instField->LongMarginRatio << "," 
				<< instField->ShortMarginRatio << "," 
				<< instField->MaxMarginSideAlgorithm << "," 
				<< instField->UnderlyingInstrID << "," 
				<< instField->StrikePrice << "," 
				<< instField->OptionsType << "," 
				<< instField->UnderlyingMultiple << "," 
				<< instField->CombinationType 
				<< endl;
	}else{
		// title
		output.open("output/instrumentListField.csv");
		output << "��Լ����" << "," 
			<< "����������" << ","
			<< "��Լ����" << "," 
			<< "��Լ�ڽ���������" << "," 
			<< "��Ʒ����" << "," 
			<< "��Ʒ����" << "," 
			<< "�������" << "," 
			<< "������" << ","
			<< "�м۵�����µ���" << "," 
			<< "�м۵���С�µ���" <<","
			<< "�޼۵�����µ���" << "," 
			<< "�޼۵���С�µ���" << "," 
			<< "��Լ��������" <<","
			<< "��С�䶯��λ"  << ","
			<< "������"  << ","
			<< "������"  << ","
			<< "������"  << ","
			<< "��ʼ������"  << ","
			<< "��Լ��������״̬"  << ","
			<< "��ǰ�Ƿ���"  << ","
			<< "�ֲ�����"  << ","
			<< "�ֲ���������"  << ","
			<< "��ͷ��֤����"  << ","
			<< "��ͷ��֤����"  << ","
			<< "�Ƿ�ʹ�ô��߱�֤���㷨"  << ","
			<< "������Ʒ����"  << ","
			<< "ִ�м�"  << ","
			<< "��Ȩ����"  << ","
			<< "��Լ������Ʒ����"  << ","
			<< "�������"  
			<< endl;
		output  << instField->InstrumentID << "," 
				<< instField->ExchangeID << "," 
				<< instField->InstrumentName << "," 
				<< instField->ExchangeInstID << "," 
				<< instField->ProductID << "," 
				<< instField->ProductClass << "," 
				<< instField->DeliveryYear << "," 
				<< instField->DeliveryMonth << "," 
				<< instField->MaxMarketOrderVolume << "," 
				<< instField->MinMarketOrderVolume << "," 
				<< instField->MaxLimitOrderVolume << "," 
				<< instField->MinLimitOrderVolume << "," 
				<< instField->VolumeMultiple << "," 
				<< instField->PriceTick << "," 
				<< instField->CreateDate << "," 
				<< instField->OpenDate << "," 
				<< instField->ExpireDate << "," 
				<< instField->StartDelivDate << "," 
				<< instField->EndDelivDate << "," 
				<< instField->InstLifePhase << "," 
				<< instField->IsTrading << "," 
				<< instField->PositionType << "," 
				<< instField->PositionDateType << "," 
				<< instField->LongMarginRatio << "," 
				<< instField->ShortMarginRatio << "," 
				<< instField->MaxMarginSideAlgorithm << "," 
				<< instField->UnderlyingInstrID << "," 
				<< instField->StrikePrice << "," 
				<< instField->OptionsType << "," 
				<< instField->UnderlyingMultiple << "," 
				<< instField->CombinationType 
				<< endl;
	}
	output.close();

}

void cTraderSpi::showPositionDetail(){
	this->m_positionCollection->PrintDetail();
}

void cTraderSpi::ReqQryInstrument()
{


	CThostFtdcQryInstrumentField req;
	memset( &req, 0, sizeof(req) );

	while( true )
	{
		int iResult = m_pUserTraderApi->ReqQryInstrument( &req, ++iRequestID );
		char message[256];
		if( !IsFlowControl( iResult ) )
		{
			if( m_genLog )
			{
				sprintf( message, "%s:called cTraderSpi::ReqQryInstrument: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail" ) );
				cout << message << endl;
				cSystem::WriteLogFile( m_logFile.c_str(), message, false );
			}
			break;
		}
		else
		{
			if( m_genLog )
			{
				sprintf( message, "%s:called cTraderSpi::ReqQryInstrument: Flow Control.", cSystem::GetCurrentTimeBuffer().c_str() );
				cout << message << endl;
				cSystem::WriteLogFile( m_logFile.c_str(), message, false );
			}
			cSystem::Sleep(1000);
		}
	} 
}

void cTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
	TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, m_brokerID);  
	strcpy(req.InvestorID, m_investorID); 	
	strcpy(req.InstrumentID, instId); 
	strcpy(req.OrderRef, m_ORDER_REF);  

	req.LimitPrice = price;	//�۸�
	if(0==req.LimitPrice){
		req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//�۸�����=�м�
		req.TimeCondition = THOST_FTDC_TC_IOC;//��Ч������:������ɣ�������
	}else{
		req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�	
		req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
	}

	req.Direction = this->MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = this->MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����

	//����Ҳ����
	/*
	req.Direction = dir;
	req.CombOffsetFlag[0] = kpp[0];
	*/

	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־	
	req.VolumeTotalOriginal = vol;	///����		
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

	//TThostFtdcPriceType	StopPrice;  //ֹ���
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��
	if(m_pUserTraderApi){
		int iResult = m_pUserTraderApi->ReqOrderInsert(&req, ++iRequestID);
		if(iResult !=0){
			char message[256];
			sprintf( message, "%s:called cTraderSpi::ReqQryInstrument: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail" ) );
			cerr << message << endl;
		}
		else
		{
			int orderRef = atoi( m_ORDER_REF );
			m_allOrderRef.push_back( orderRef );
			sprintf( m_ORDER_REF, "%d", ++orderRef );
		}
	}

}

void cTraderSpi::ReqOrderInsert( cOrder* pOrder )
{
	CThostFtdcInputOrderField req;

	memset( &req, 0, sizeof( req ) );
	
	strcpy( req.BrokerID, m_brokerID );
	strcpy( req.InvestorID, m_investorID );
	strcpy( req.UserID, m_investorID );
	strcpy( req.InstrumentID, pOrder->GetInstrumentID().c_str() );
	strcpy( req.OrderRef, m_ORDER_REF );

	req.Direction = pOrder->GetDirection();

	req.CombOffsetFlag[0] = pOrder->GetOffsetFlag();

	req.VolumeTotalOriginal = pOrder->GetVolumeOriginal();

	// order with market price
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;  
	req.LimitPrice = pOrder->GetPrice();  
	req.TimeCondition = THOST_FTDC_TC_GFD;
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	req.VolumeCondition = THOST_FTDC_VC_AV;
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.MinVolume = 1;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = 0;
	req.UserForceClose = 0;

	int iResult = m_pUserTraderApi->ReqOrderInsert( &req, ++iRequestID );
	//cout << "--->>> ReqOrderInsert: " << iResult << ( ( iResult == 0 ) ? ", succeed" : ", fail") << endl;
	if( iResult == 0 )
	{
		int orderRef = atoi( m_ORDER_REF );
		m_allOrderRef.push_back( orderRef );
		sprintf( m_ORDER_REF, "%d", ++orderRef );
		pOrder = NULL;
	}
}

// order insertion response
void cTraderSpi::OnRspOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	//output the order insertion result
	//it seems that this function is called only when error occurs during inserting orders
	
	if( m_genLog )
	{
		printf( "ErrorCode = [%d], ErrorMsg = [%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg );
		char message[256];
		sprintf( message, "%s:called cTraderSpi::OnRspOrderInsert.", cSystem::GetCurrentTimeBuffer().c_str() );
		cout << message << endl;
		cSystem::WriteLogFile( m_logFile.c_str(), message, false );
	}

	if( !IsErrorRspInfo( pRspInfo ) )
	{
	}
}

// order insertion return
void cTraderSpi::OnRtnOrder( CThostFtdcOrderField* pOrder )
{
	if(pOrder && !strcmp(pOrder->InsertDate,this->m_investorID)){
		m_orderCollection->Add( pOrder );
		if( !IsMyOrder( pOrder ) ){
			cerr << " Other:";
		}else{
			cerr<< " My";
		}
		cerr <<" No" <<pOrder->OrderSysID <<"  Status:" << pOrder->OrderStatus << pOrder->StatusMsg << endl;
	}

}



void cTraderSpi::OnRspOrderAction( CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	/* calling this function in case the order cancellation is failed */
	if( !IsErrorRspInfo( pRspInfo ) )
	{
		cout << "OnRspOrderAction" << endl;
	}


}


// transaction notification
// update m_positions
void cTraderSpi::OnRtnTrade( CThostFtdcTradeField* pTrade )
{
	/* update of m_tradeCollection */
	auto iter = (this->m_pInstCommissionMap->find(pTrade->InstrumentID) == this->m_pInstCommissionMap->end()? NULL : this->m_pInstCommissionMap->at(pTrade->InstrumentID));
	m_tradeCollection->Add( pTrade,iter.get(),m_InstMeassageMap->at(pTrade->InstrumentID));
	int tradeID = atoi( pTrade->TradeID );
	m_tradeCollection->PrintTrade( tradeID );

	///* update of m_positionDetail */
	m_positionCollection->update( pTrade );

	//subscirbe Instrument 
	this->subscribeInst(pTrade->InstrumentID,true);

	// 
	for each (auto var in m_strategyList)
	{
		var->onTrade(*pTrade);
	}
}






// the error notification caused by client request
void cTraderSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast )
{
	printf( "OnRspError:\n" );
	printf( "ErrorCode = [%d], ErrorMsg = [%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg );
	printf( "RequestID = [%d], Chain = [%d]\n", nRequestID, bIsLast );

	// the client should handle the error
}

void cTraderSpi::OnHeartBeatWarning( int nTimeLapse )
{
	cout << "--->>> " << "OnHeartBeatWarning" << endl;
	cout << "--->>> nTimerLapse = " << nTimeLapse << endl;
}


// ReqOrderAction is used for order cancellation
void cTraderSpi::ReqOrderAction(shared_ptr<cOrder>  pOrder )
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));

	strcpy( req.BrokerID, m_brokerID );
	strcpy( req.InvestorID, m_investorID );
	//sprintf( req.OrderRef, "%d",pOrder->GetOrderRef() );
	strcpy( req.OrderSysID, pOrder->m_orderSysID );
	strcpy( req.ExchangeID, pOrder->ExchangeID );
	req.FrontID = m_FRONT_ID;
	req.SessionID = m_SESSION_ID;
	req.ActionFlag = THOST_FTDC_AF_Delete;

	int iResult = m_pUserTraderApi->ReqOrderAction( &req, ++iRequestID );

	/*char message[256];
	sprintf( message, "%s:called cTraderSpi::ReqOrderAction: %s.", cSystem::GetCurrentTimeBuffer().c_str(), ( ( iResult == 0 ) ? "Success" : "Fail" ) );
	cout << message << endl;*/
}



bool cTraderSpi::IsErrorRspInfo( CThostFtdcRspInfoField* pRspInfo )
{
	bool bResult = ( ( pRspInfo ) && ( pRspInfo->ErrorID != 0 ) );
	if (bResult)
		printf( "  ErrorCode = [%d], ErrorMsg = [%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg );
	return bResult;
}

bool cTraderSpi::IsMyOrder( CThostFtdcOrderField* pOrder )
{
	bool flag = false;

	int orderRef = atoi( pOrder->OrderRef );

	for( size_t i = 0; i < m_allOrderRef.size(); ++i )
	{
		if( orderRef == m_allOrderRef[i] )
		{
			flag = true;
			break;
		}
	}

	return( flag && ( pOrder->FrontID == m_FRONT_ID ) && ( pOrder->SessionID == m_SESSION_ID ) );
}


void cTraderSpi::GetInstrumentIDs( cArray< cString >& instrumentIDs ) const
{
	for( int i = 0; i < m_instrumentIDs.getSize(); ++i )
		instrumentIDs.push_back( m_instrumentIDs[i] );
}

const sInstrumentInfo* cTraderSpi::GetInstrumentInfo( const cString& instrumentID ) const
{
	map< cString, sInstrumentInfo* >::const_iterator it = m_instrumentInfo.find( instrumentID );
	if( it != m_instrumentInfo.end() )
		return (*it).second;
	else
		yr_error( "instrument %s not found!", instrumentID.c_str() );
}


void cTraderSpi::RegisterPositionCollection( cPositionCollectionPtr p ) 
{
	if( m_positionCollection.get() )
		m_positionCollection.reset();

	m_positionCollection = p; 
}

void cTraderSpi::RegisterOrderCollection( cOrderCollectionPtr p ) 
{ 
	if( m_orderCollection.get() )
		m_orderCollection.reset();

	m_orderCollection = p; 
}

void cTraderSpi::RegisterTradeCollection( cTradeCollectionPtr p )
{ 
	if( m_tradeCollection.get() )
		m_tradeCollection.reset();

	m_tradeCollection = p; 
}
void cTraderSpi::RegisterSubscribeInstList(shared_ptr<vector<string>> p){
	if( m_pSubscribeInst.get() )
		m_pSubscribeInst.reset();

	m_pSubscribeInst = p; 
}
void cTraderSpi::RegisterInstMessageMap( map<string, CThostFtdcInstrumentField*>* p )
{ 
	if( m_InstMeassageMap )
		m_InstMeassageMap = NULL;

	m_InstMeassageMap = p; 
}
void cTraderSpi::RegisterInstCommissionMap( map<string,shared_ptr< CThostFtdcInstrumentCommissionRateField>>*p )
{ 
	if( m_pInstCommissionMap )
		m_pInstCommissionMap = NULL;

	m_pInstCommissionMap = p; 
}
void cTraderSpi::insertOrder(string inst,DIRECTION dire,OFFSETFLAG flag, int vol,double orderPrice,int priceTick ){
	TThostFtdcInstrumentIDType    instId;
	TThostFtdcDirectionType       dir;//����,'0'��'1'��
	TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
	TThostFtdcPriceType           price;//�۸�0���м�,��������֧��
	strcpy(instId,inst.c_str());

	//double miniChangeTick = m_instMessage_map[inst.c_str()]->PriceTick * 3; // ������ ��С�䶯�۸� ��֤�ɽ�
	double BuyPrice = orderPrice + priceTick * this->m_InstMeassageMap->at(inst)->PriceTick;;
	double SellPrice = orderPrice - priceTick * this->m_InstMeassageMap->at(inst)->PriceTick;;// ������ �����
	// make market price order
	if(orderPrice == 0){
		cMarketData *p;
		double lastprice = 0;
		p = this->m_pMarketDataEngine->GetMarketDataHandle(inst);
		if(p) {
			lastprice = p->getLastMarketData().LastPrice;
		}else{
			cerr << "Inst Error" << endl;
			this->m_pMdSpi->SubscribeMarketData(inst);
			return;
		}
		switch (dire)
		{
		case buy:
			BuyPrice = lastprice + (1 + priceTick) * this->m_InstMeassageMap->at(inst)->PriceTick;
			break;
		case sell:
			SellPrice =  lastprice - (1 + priceTick) * this->m_InstMeassageMap->at(inst)->PriceTick;
			break;
		}
	}

	
	//����
	if(flag == OFFSETFLAG::open){
		//���뿪��
		if( dire == DIRECTION::buy){
			dir = '0';
			strcpy_s(kpp, "0");
			price = BuyPrice;
			this->ReqOrderInsert(instId, dir, kpp, price, vol);
		}
		// ��������
		if( dire == DIRECTION::sell){
			dir = '1';
			strcpy_s(kpp, "0");
			price = SellPrice;
			this->ReqOrderInsert(instId, dir, kpp, price, vol);
			
		}
	}
	if(flag == OFFSETFLAG::close){
	
		//����ƽ��
		if(dire==DIRECTION::buy){
			dir = '0';
			price = BuyPrice;
			this->StraitClose(instId, dir, price, vol);
		}
		//����ƽ��
		if(dire==DIRECTION::sell){
			dir = '1';
			price = SellPrice ;
			this->StraitClose(instId, dir, price, vol);
		}
	}

}
void cTraderSpi::StraitClose(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcPriceType price,TThostFtdcVolumeType vol){
	TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ�� ƽ,"3"ƽ��
	// close Long
	if(dir == '1'){
		if(this->m_positionCollection->getHolding_long(instId) < vol){
			cerr << "Long:close vol more than hold vol" << endl;
			return;
		}
		if(this->m_positionCollection->getHolding_long(instId)> 0 )
		{
			if(strcmp(m_InstMeassageMap->at(instId)->ExchangeID, "SHFE") == 0)
			{
				// close y than close t
				int Yd_long =this->m_positionCollection->getYdLong(instId);
				int Td_long =this->m_positionCollection->getTdLong(instId);
				if(Yd_long > vol)
				{
					strcpy(kpp, "1");// close yP
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				else
				{
					if(Yd_long>0){
						strcpy(kpp, "1");//close yP
						ReqOrderInsert(instId, dir, kpp, price, Yd_long);
					}
					strcpy(kpp, "3");//close tP
					ReqOrderInsert(instId, dir, kpp, price, vol-Yd_long);
				}

			}
			else
			{
				strcpy(kpp, "1");
				ReqOrderInsert(instId, dir, kpp, price, vol);
				
			}


		}
	}
	//close short
	else{
		if(this->m_positionCollection->getHolding_short(instId)< vol){
			cerr << "short:close vol more than hold vol" << endl;
			return ;
		}
		
		if(this->m_positionCollection->getHolding_short(instId) > 0)
		{
			if(strcmp(m_InstMeassageMap->at(instId)->ExchangeID, "SHFE") == 0)
			{
				// close yP than tP
				int Yd_short = this->m_positionCollection->getYdShort(instId);
				int Td_short = this->m_positionCollection->getTdShort(instId);

				if(Yd_short >= vol)
				{
					strcpy(kpp, "1");//yP
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				else// yd is not enough
				{
					if(Yd_short>0){
						strcpy(kpp, "1");//yP
						ReqOrderInsert(instId, dir, kpp, price, Yd_short);
					}
					strcpy(kpp, "3");//tP
					ReqOrderInsert(instId, dir, kpp, price, vol-Yd_short);
				}

			}
			else
			{
				strcpy(kpp, "1");
				ReqOrderInsert(instId, dir, kpp, price, vol);
				
			}



		}
	
	}

}
char cTraderSpi::MapDirection(char src, bool toOrig=true){
	if(toOrig){
		if('b'==src||'B'==src){src='0';}else if('s'==src||'S'==src){src='1';}
	}else{
		if('0'==src){src='B';}else if('1'==src){src='S';}
	}
	return src;
}


char cTraderSpi::MapOffset(char src, bool toOrig=true){
	if(toOrig){
		if('o'==src||'O'==src){src='0';}
		else if('c'==src||'C'==src){src='1';}
		else if('j'==src||'J'==src){src='3';}
	}else{
		if('0'==src){src='O';}
		else if('1'==src){src='C';}
		else if('3'==src){src='J';}
	}
	return src;
}

bool cTraderSpi::subscribeInst(TThostFtdcInstrumentIDType instrumentName,bool subscribeTag){
	bool find_instId_Trade = false;

	for(unsigned int i = 0; i< m_pSubscribeInst->size(); i++)
	{
		if(strcmp(m_pSubscribeInst->at(i).c_str(), instrumentName) == 0)/// already subscribe Instrument 
		{
			find_instId_Trade = true;
			break;
		}
	}

	if(!find_instId_Trade)
	{	
		if(subscribeTag){
			m_pMdSpi->SubscribeMarketData(instrumentName);
		}
		m_pSubscribeInst->push_back(instrumentName); 
	}
	return true;
}


bool cTraderSpi::isValidInsturment(string inst,string& instName){

	std::regex pattern("[a-zA-Z]{1,2}[0-9]{3,4}");

	std::match_results<std::string::const_iterator> result;

	bool valid = regex_match(inst, result, pattern);
	char instCName[3]= {};
	if(valid){
		sscanf(inst.c_str(),"%[a-zA-Z]{1,2}",instCName);
	}
	instName = string(instCName);
	return valid;

}

void cTraderSpi::cancleAllPendingOrder(){
	vector<cOrderPtr> vOrder = this->m_orderCollection->GetAllOrder();
	for(auto it = vOrder.begin();it!=vOrder.end();it++){
		if(it->get()->IsPendingOrder() ){
			this->ReqOrderAction(*it);
		}
	}
}

void cTraderSpi::cancleMyPendingOrder(){
	vector<cOrderPtr> vOrder = this->m_orderCollection->GetAllOrder();
	for(auto it = vOrder.begin();it!=vOrder.end();it++){
		if(it->get()->IsPendingOrder() && IsMyOrder( it->get()->getOrderStruct() ) ){
			this->ReqOrderAction(*it);
		}
	}
}