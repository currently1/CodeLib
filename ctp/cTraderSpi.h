#ifndef __CTRADERSPI_H__
#define __CTRADERSPI_H__

#include <stl_ctp.h>
#include <map>
#include <yr_structs.h>
#include <cPositionCollection.h>
#include <cTradeCollection.h>
#include <cOrderCollection.h>
#include <cSignal.h>

#ifndef _DEBUG
#define _DEBUG 0
#endif
class cString;
template< class T > class cArray;

class cTraderSpi : public CThostFtdcTraderSpi
{
public:
	cTraderSpi( CThostFtdcTraderApi* pUserTraderApi, CThostFtdcMdApi* pUserMdApi,TThostFtdcBrokerIDType brokerID, TThostFtdcInvestorIDType investorID, TThostFtdcPasswordType password, bool genLog = false );
	
	~cTraderSpi();

	// After making a succeed connection with the CTP server, the client should send the login request to the CTP server.
	virtual void OnFrontConnected();

	// When the connection between client and the CTP server disconnected, the following function will be called
	virtual void OnFrontDisconnected(int nReason);

	// After receiving the login request from the client, the CTP server will send the following response to notify the client whether the login success or not
	virtual void OnRspUserLogin( CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// investor settlement information confirmation response
	virtual void OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// query instrument response
	virtual void OnRspQryInstrument( CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// query trading account response
	virtual void OnRspQryTradingAccount( CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// After receiving the investor position request, the CTP server will send the following response to notify the client whether the request success or not
	//virtual void OnRspQryInvestorPosition( CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );
	
	// After receiving the investor position detail request, the CTP server will send the following response to notify the client whether the request success or not
	virtual void OnRspQryInvestorPositionDetail( CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// order insertion response
	virtual void OnRspOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	// order action response
	virtual void OnRspOrderAction( CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ); 

	// order query response
	virtual void OnRspQryOrder( CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );
	
	// the error notification caused by client request
	virtual void OnRspError( CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	virtual void OnHeartBeatWarning( int nTimeLapse );

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	// order insertion return
	virtual void OnRtnOrder( CThostFtdcOrderField* pOrder );

	virtual void OnRtnTrade( CThostFtdcTradeField* pTrade);

	void RegisterPositionCollection( cPositionCollectionPtr p );
	void RegisterOrderCollection( cOrderCollectionPtr p );
	void RegisterTradeCollection( cTradeCollectionPtr p );
	void RegisterInstMessageMap( cInstMessageMapPtr p );
	void ReqQryInstrument();

	void ReqQryInstrument_all();

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition_all();

	//void ReqQryInvestorPosition();
	void ReqQryInvestorPositionDetail();

	void ReqQryOrder();

	void ReqQryTrade();

	void ReqOrderInsert( cOrder* pOrder );

	void ReqOrderAction( cOrder* pOrder );

	void Close();

	void Init();

	void GetInstrumentIDs( cArray< cString >& ) const;

	const sInstrumentInfo* GetInstrumentInfo( const cString& ) const;

	const sTradingAccountInfo* GetTradingAccountInfo() const { return m_accountInfo; }

	void cTraderSpi::saveInstrumentField(CThostFtdcInstrumentField* instField);

	void showPositionDetail();
	void ReqOrderInsert(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,   TThostFtdcVolumeType vol);
	// insert order
	void insertOrder(string inst,DIRECTION dire,OFFSETFLAG flag, int vol,double orderPrice);

	void StraitClose(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcPriceType price,TThostFtdcVolumeType vol);
	 
private:
	CThostFtdcTraderApi* m_pUserTraderApi;
	cArray< cString > m_instrumentIDs;

	TThostFtdcOrderRefType	m_ORDER_REF;
	TThostFtdcFrontIDType	m_FRONT_ID;
	TThostFtdcSessionIDType	m_SESSION_ID;
	
	sTradingAccountInfo* m_accountInfo;
	map< cString, sInstrumentInfo* > m_instrumentInfo;		// useful trading information for traded instruments
	//
	/* postions */
	/*cPositionCollection* m_positionCollection;*/
	cPositionCollectionPtr m_positionCollection;
	// 
	/* orders */
	/*cOrderCollection* m_orderCollection;*/
	cOrderCollectionPtr m_orderCollection;

	cIvector m_allOrderRef;											// list of all orderRef
	//
	/* trades */
	/*cTradeCollection* m_tradeCollection;*/
	cTradeCollectionPtr m_tradeCollection;

	// Instrument detail Message Map
	cInstMessageMapPtr m_InstMeassageMap;

	void ReqUserLogin();
	void ReqSettlementInfoConfirm();
	bool IsErrorRspInfo( CThostFtdcRspInfoField* pRspInfo );
	bool IsMyOrder( CThostFtdcOrderField* pOrder );

	TThostFtdcBrokerIDType	m_brokerID;
	TThostFtdcInvestorIDType m_investorID;
	TThostFtdcPasswordType	m_password;

	bool m_genLog;
	cString m_logFile;

	//=======================20170828==================
	bool m_first_inquiry_order;//�Ƿ��״β�ѯ����
	bool m_first_inquiry_trade;//�Ƿ��״β�ѯ�ɽ�
	bool m_firs_inquiry_Detail;//�Ƿ��״β�ѯ�ֲ���ϸ
	bool m_firs_inquiry_TradingAccount;//�Ƿ��״β�ѯ�ʽ��˺�
	bool m_firs_inquiry_Position;//�Ƿ��״β�ѯͶ���ֲ߳�
	bool m_first_inquiry_Instrument;//�Ƿ��״β�ѯ��Լ
	
	vector<CThostFtdcOrderField*> m_orderList;//ί�м�¼��ȫ����Լ
	vector<CThostFtdcOrderField*> m_pendOrderList;//�ҵ���¼��ȫ����Լ
    vector<CThostFtdcTradeField*> m_tradeList;//�ɽ���¼��ȫ����Լ

	vector<CThostFtdcTradeField*> m_tradeListNotClosedAccount;//δƽ�ּ�¼

	map<string,cPositionDetailPtr> m_position_message_map;//�ֲּ�¼ 

	double m_closeProfit;//ƽ��ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ��ƽ��ӯ��ֵ
	
	double m_OpenProfit;//����ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ�ĸ���ӯ��ֵ

	//map<string, CThostFtdcInstrumentField*> m_instMessage_map;//�����Լ��Ϣ��map
	
	CThostFtdcMdApi* m_pMDUserApi_td;//����APIָ�룬���캯���︳ֵ
	
	double m_accountMargin;
};


#endif
