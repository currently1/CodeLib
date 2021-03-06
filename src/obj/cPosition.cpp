#include <cPosition.h>
#include <cTrade.h>

cPositionDetail::cPositionDetail(string inst)
{
	memset(this,0,sizeof(this));

	m_lastPrice = 0;//最新价，时刻保存合约的最新价，平仓用
	m_PreSettlementPrice = 0;//上次结算价，对隔夜仓有时候要用，快期有用
	m_holding_long = 0;//多单持仓量
	m_holding_short = 0;//空单持仓量

	m_TodayPosition_long = 0;//多单今日持仓
	m_YdPosition_long = 0;//多单上日持仓

	m_TodayPosition_short = 0;//空单今日持仓
	m_YdPosition_short = 0;//空单上日持仓

	closeProfit_long = 0;//多单平仓盈亏
	OpenProfit_long = 0;//多单浮动盈亏

	closeProfit_short = 0;//空单平仓盈亏
	OpenProfit_short = 0;//空单浮动盈亏

	margin = 0;// 持仓占用保证金
	this->m_instrumentID = inst;

}



void cPositionDetail::update( CThostFtdcInvestorPositionField* pInvestorPosition )
{

	if(strcmp(pInvestorPosition->InstrumentID,this->m_instrumentID.c_str()) == 0){
		if(pInvestorPosition->PosiDirection == '2')//多单
		{
			//多单持仓量
			this->m_holding_long += pInvestorPosition->Position;

			//多单今仓
			this->m_TodayPosition_long = pInvestorPosition->TodayPosition;

			//多单昨仓 = 多单持仓量 - 多单今仓
			this->m_YdPosition_long = this->m_holding_long - this->m_TodayPosition_long;//也可以
			//m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long = pInvestorPosition->Position - pInvestorPosition->TodayPosition;

			//多单平仓盈亏
			this->closeProfit_long =  pInvestorPosition->CloseProfit;

			//多单浮动盈亏(其实是持仓盈亏，按昨结算的)
			this->OpenProfit_long = pInvestorPosition->PositionProfit;

			this->margin = pInvestorPosition->UseMargin;

		}
		else if(pInvestorPosition->PosiDirection == '3')//空单
		{
			//空单持仓量
			this->m_holding_short += pInvestorPosition->Position;

			//空单今仓
			this->m_TodayPosition_short = pInvestorPosition->TodayPosition;

			//空单昨仓 = 空单持仓量 - 空单今仓
			this->m_YdPosition_short = this->m_holding_short - this->m_TodayPosition_short;

			//空单平仓盈亏
			this->closeProfit_short = pInvestorPosition->CloseProfit;

			//空单持仓盈亏
			this->OpenProfit_short = pInvestorPosition->PositionProfit;

			this->margin = pInvestorPosition->UseMargin;
		}
	}else{
		yr_error("cPositionDetail update error");
	}
}

void cPositionDetail::update( CThostFtdcTradeField* pTrade )
{

	if(strcmp(pTrade->InstrumentID,this->m_instrumentID.c_str()) == 0)
	{
		if (pTrade->OffsetFlag == THOST_FTDC_OF_Open)//开仓
		{
			if (pTrade->Direction == THOST_FTDC_D_Buy)//多单
			{

				//多单持仓量
				this->m_holding_long = this->m_holding_long + pTrade->Volume;
				//多单今日持仓
				this->m_TodayPosition_long = this->m_TodayPosition_long + pTrade->Volume;
			}
			else if (pTrade->Direction == THOST_FTDC_D_Sell)//空单
			{									

				//空单持仓量
				this->m_holding_short =this ->m_holding_short + pTrade->Volume;
				//空单今日持仓
				this->m_TodayPosition_short = this->m_TodayPosition_short + pTrade->Volume;
			}
		}
		else 
		{
			if (pTrade->Direction == THOST_FTDC_D_Sell)//卖，表示平多,有昨仓和今仓时，按时间顺序，先平昨仓
			{


				//多单持仓量
				this->m_holding_long =this->m_holding_long - pTrade->Volume;
				//今仓持仓量和昨仓量，要分上期所和非上期所


				//今仓量和昨仓量，只对上期所有效
				if (pTrade->OffsetFlag == THOST_FTDC_OF_Close || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday || pTrade->OffsetFlag == THOST_FTDC_OF_ForceClose)
					this->m_YdPosition_long =this->m_YdPosition_long - pTrade->Volume;//昨仓
				else if (pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday)
					this->m_TodayPosition_long =this->m_TodayPosition_long - pTrade->Volume;//今仓


				//假设今仓5手，昨仓1，平仓都是发'1'，假设平仓2手，导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0
				//3手昨仓，5手今仓，，'1'平仓了4手,导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0

				if(this->m_YdPosition_long < 0)
				{
					this->m_TodayPosition_long =this->m_TodayPosition_long + this->m_YdPosition_long;
					this->m_YdPosition_long = 0;

				}
			}
			else if (pTrade->Direction == THOST_FTDC_D_Buy)//平空
			{

				//空单持仓量
				this->m_holding_short = this->m_holding_short - pTrade->Volume;

				//空单今日持仓
				//this->TodayPosition_short = this->TodayPosition_short - pTrade->Volume;

				//今仓量和昨仓量，只对上期所有效
				if (pTrade->OffsetFlag == THOST_FTDC_OF_Close || pTrade->OffsetFlag == THOST_FTDC_OF_ForceClose || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday)
					this->m_YdPosition_short = this->m_YdPosition_short - pTrade->Volume;//昨仓
				else if (pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday)
					this->m_TodayPosition_short = this->m_TodayPosition_short - pTrade->Volume;//今仓

				if(this->m_YdPosition_short < 0)
				{
					this->m_TodayPosition_short = this->m_TodayPosition_short + this->m_YdPosition_short;
					this->m_YdPosition_short = 0;

				}
			}

		}
	}
	else{
		yr_error("cPositionDetail update error");
	}
}


void cPositionDetail::Print()
{
	/*printf( " Instrument:%s", m_instrumentID.c_str() );
	printf( " B/S:%s", m_direction == '0' ? "B" : "S" );
	printf( " Volume:%d", m_volume );
	printf( " OpenPrice:%5.3f", m_price );
	printf( " OpenDate:%s", m_openDate.DateToString().c_str() );
	printf( " Type:%s", m_isToday ? "PT" : "PY" );
	printf( " TradeID:%d", m_tradeID );
	printf( "\n" );
*/
	cerr<<this->m_instrumentID
	<<"\t L:"<<this->m_holding_long
	<<"\t S:"<<this->m_holding_short
	<<"\t tL:"<<this->m_TodayPosition_long
	<<"\t yL:"<<this->m_YdPosition_long
	<<"\t tS:"<<this->m_TodayPosition_short
	<<"\t yS:"<<this->m_YdPosition_short
	<<"\t L Close Profit:"<<this->closeProfit_long
	<<"\t L Hold  Profit:"<<this->OpenProfit_long
	<<"\t S Close Profit:"<<this->closeProfit_short
	<<"\t S Hold  Profit:"<<this->OpenProfit_short
	<<"\t margin:"<<this->margin
	<<endl;
}