#ifndef __CMDSPI_H__
#define __CMDSPI_H__

#include <ThostFtdcMdApi.h>
#include <ThostFtdcTraderApi.h>
#include <cString.h>
#include <memory>
using std::shared_ptr;

class cMarketDataCollection;
extern int iRequestID;

class cMdSpi : public CThostFtdcMdSpi
{
public:
	cMdSpi( CThostFtdcMdApi* pUserMdApi,  TThostFtdcBrokerIDType brokerID, TThostFtdcInvestorIDType investorID, TThostFtdcPasswordType password, bool genLog = false );

	///错误应答
	virtual void OnRspError( CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast );

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected( int nReason );
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning( int nTimeLapse );

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	
	///登录请求响应
	virtual void OnRspUserLogin( CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	///订阅行情应答
	virtual void OnRspSubMarketData( CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	///订阅询价应答
	virtual void OnRspSubForQuoteRsp( CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData( CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	///取消订阅询价应答
	virtual void OnRspUnSubForQuoteRsp( CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast );

	///深度行情通知
	virtual void OnRtnDepthMarketData( CThostFtdcDepthMarketDataField* pDepthMarketData );

	///询价通知
	virtual void OnRtnForQuoteRsp( CThostFtdcForQuoteRspField* pForQuoteRsp );

	void RegisterMarketDataCollection( cMarketDataCollection* pMktDataCollection );

	bool getSatus(){return this->m_status;}

	void SubscribeMarketData(char *instIdList);
	void SubscribeMarketData(shared_ptr<vector<string>> instList);
	void SubscribeMarketData(string inst);
private:
	void ReqUserLogin();

	void SubscribeForQuoteRsp();
	bool IsErrorRspInfo( CThostFtdcRspInfoField* pRspInfo );

	TThostFtdcBrokerIDType	m_brokerID;
	TThostFtdcInvestorIDType m_investorID;
	char	m_password[252];

	CThostFtdcMdApi* m_pUserMdApi;

	cMarketDataCollection* m_pMktDataCollection;
	
	bool m_genLog;
	
	int m_requestID;
	cString m_outputDirectory;
	cString m_logFileFolder;
	cString m_logFile;

	bool m_status;

};
typedef int (*ccbf_secureApi_LoginMd)(CThostFtdcMdApi* ctp_futures_pMdApi, TThostFtdcBrokerIDType brokeId, TThostFtdcUserIDType userId, char* pChar_passwd, int& ctp_futures_requestId);
#endif
