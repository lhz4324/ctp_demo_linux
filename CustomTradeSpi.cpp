#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include "CustomTradeSpi.h"

#include <sstream>
#include<iomanip>
#include <iostream>
#include <fstream>
#include <string>





// ---- ȫ�ֲ������� ---- //
extern TThostFtdcBrokerIDType gBrokerID;                      // ģ�⾭���̴���
extern TThostFtdcInvestorIDType gInvesterID;                  // Ͷ�����˻���
extern TThostFtdcPasswordType gInvesterPassword;              // Ͷ��������
extern CThostFtdcTraderApi* g_pTradeUserApi;                  // ����ָ��
extern char gTradeFrontAddr[];                                // ģ�⽻��ǰ�õ�ַ
extern TThostFtdcInstrumentIDType g_pTradeInstrumentID;       // �����׵ĺ�Լ����
extern TThostFtdcDirectionType gTradeDirection;               // ��������
extern TThostFtdcPriceType gLimitPrice;                       // ���׼۸�

// �Ự����
TThostFtdcFrontIDType	trade_front_id;	//ǰ�ñ��
TThostFtdcSessionIDType	session_id;	//�Ự���
TThostFtdcOrderRefType	order_ref;	//��������
time_t lOrderTime;
time_t lOrderOkTime;



CustomTradeSpi::CustomTradeSpi(const std::string& username,int contractAlertNum, int allContractAlertNum)
{	

	this->accountname = username;

	this->ContractAlertNum = contractAlertNum;
	this->AllContractAlertNum = allContractAlertNum;
	std::string mylog = "account_" + username;
	this->m_logger= spdlog::basic_logger_mt(mylog, "log.txt");
}

void CustomTradeSpi::OnFrontConnected()
{
	std::unique_lock<std::mutex> unique(mt);
	std::cout << "=====�����������ӳɹ�=====" << std::endl;

	m_logger->info("=====�����������ӳɹ�=====");
	m_logger->flush();
	// ��ʼ��¼
	unique.unlock();
	reqUserLogin();
}

void CustomTradeSpi::OnRspUserLogin(
	CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{

	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << "=====�˻���¼�ɹ�=====" << std::endl;
		loginFlag = true;
		std::cout << "�����գ� " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "��¼ʱ�䣺 " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "�����̣� " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "�ʻ����� " << pRspUserLogin->UserID << std::endl;
		std::cout << "------------------------------------------------" << std::endl;


		m_logger->info("�˻���¼�ɹ��� �����գ�{},��¼ʱ�䣺{},�����̣�{},�ʻ�����{}",
			pRspUserLogin->TradingDay, pRspUserLogin->LoginTime, pRspUserLogin->BrokerID, pRspUserLogin->UserID);

		// ����Ự����
		trade_front_id = pRspUserLogin->FrontID;
		session_id = pRspUserLogin->SessionID;
		strcpy(order_ref, pRspUserLogin->MaxOrderRef);

		unique.unlock();

		// Ͷ���߽�����ȷ��
		//reqSettlementInfoConfirm();

		//��ѯ����
		reqQryOrder();

	}
	else {
		std::cout << "=====�˻���¼ʧ��=====" << std::endl;
		m_logger->error("�˻���¼ʧ��");
	}

	m_logger->flush();

}

void CustomTradeSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);
}

void CustomTradeSpi::OnFrontDisconnected(int nReason)
{
	m_logger->error("�������ӶϿ�,�����룺{}", nReason);
	m_logger->flush();
}

void CustomTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
	m_logger->error("����������ʱ,���ϴ�����ʱ�䣺{}", nTimeLapse);
	m_logger->flush();
}

void CustomTradeSpi::OnRspUserLogout(
	CThostFtdcUserLogoutField* pUserLogout,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		loginFlag = false; // �ǳ��Ͳ����ٽ����� 
		//std::cout << "=====�˻��ǳ��ɹ�=====" << std::endl;
		//std::cout << "�����̣� " << pUserLogout->BrokerID << std::endl;
		//std::cout << "�ʻ����� " << pUserLogout->UserID << std::endl;

		m_logger->info("�˻��ǳ��ɹ�,�����̣�{},�ʻ�����{}", pUserLogout->BrokerID, pUserLogout->UserID);
		m_logger->flush();
	}
}

void CustomTradeSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		//std::cout << "=====Ͷ���߽�����ȷ�ϳɹ�=====" << std::endl;
		//std::cout << "ȷ�����ڣ� " << pSettlementInfoConfirm->ConfirmDate << std::endl;
		//std::cout << "ȷ��ʱ�䣺 " << pSettlementInfoConfirm->ConfirmTime << std::endl;

		unique.unlock();

		// �����ѯ��Լ
		//reqQueryInstrument();

	}

}

void CustomTradeSpi::OnRspQryInstrument(
	CThostFtdcInstrumentField* pInstrument,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		//std::cout << "=====��ѯ��Լ����ɹ�=====" << std::endl;
		//std::cout << "���������룺 " << pInstrument->ExchangeID << std::endl;
		//std::cout << "��Լ���룺 " << pInstrument->InstrumentID << std::endl;
		//std::cout << "��Լ�ڽ������Ĵ��룺 " << pInstrument->ExchangeInstID << std::endl;
		//std::cout << "ִ�мۣ� " << pInstrument->StrikePrice << std::endl;
		//std::cout << "�����գ� " << pInstrument->EndDelivDate << std::endl;
		//std::cout << "��ǰ����״̬�� " << pInstrument->IsTrading << std::endl;

		unique.unlock();
		// �����ѯͶ�����ʽ��˻�
		//reqQueryTradingAccount();
	}
}

void CustomTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField* pTradingAccount,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		//std::cout << "=====��ѯͶ�����ʽ��˻��ɹ�=====" << std::endl;
		//std::cout << "Ͷ�����˺ţ� " << pTradingAccount->AccountID << std::endl;
		//std::cout << "�����ʽ� " << pTradingAccount->Available << std::endl;
		//std::cout << "��ȡ�ʽ� " << pTradingAccount->WithdrawQuota << std::endl;
		//std::cout << "��ǰ��֤��: " << pTradingAccount->CurrMargin << std::endl;
		//std::cout << "ƽ��ӯ���� " << pTradingAccount->CloseProfit << std::endl;

		unique.unlock();
		// �����ѯͶ���ֲ߳�
		//reqQueryInvestorPosition();
	}
}

void CustomTradeSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		//std::cout << "=====��ѯͶ���ֲֳ߳ɹ�=====" << std::endl;
		if (pInvestorPosition)
		{
			//std::cout << "��Լ���룺 " << pInvestorPosition->InstrumentID << std::endl;
			//std::cout << "���ּ۸� " << pInvestorPosition->OpenAmount << std::endl;
			//std::cout << "�������� " << pInvestorPosition->OpenVolume << std::endl;
			//std::cout << "���ַ��� " << pInvestorPosition->PosiDirection << std::endl;
			//std::cout << "ռ�ñ�֤��" << pInvestorPosition->UseMargin << std::endl;
		}
		else
			//std::cout << "----->�ú�Լδ�ֲ�" << std::endl;

			unique.unlock();


		// ����¼������������һ���ӿڣ��˴��ǰ�˳��ִ�У�
		if (loginFlag)
		{
			//reqOrderInsert();
			//reqQryOrder();
		}


		//if (loginFlag)
		//	reqOrderInsertWithParams(g_pTradeInstrumentID, gLimitPrice, 1, gTradeDirection); // �Զ���һ�ʽ���

		// ���Խ���
		//std::cout << "=====��ʼ������Խ���=====" << std::endl;
		//while (loginFlag)
		//	StrategyCheckAndTrade(g_pTradeInstrumentID, this);
	}
}


void CustomTradeSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo)&& pOrder != nullptr)
	{
		std::unique_lock<std::mutex> unique(mt);
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << "=====�����ѯ�����ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pOrder->InstrumentID << std::endl;
		std::cout << "�۸� " << pOrder->LimitPrice << std::endl;
		std::cout << "������ " << pOrder->VolumeTotalOriginal << std::endl;
		std::cout << "���ַ��� " << pOrder->Direction << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->info("�����ѯ�����ɹ�!��Լ���룺{},�۸�{},������{},���ַ���{}", pOrder->InstrumentID, pOrder->LimitPrice,
			pOrder->VolumeTotalOriginal, pOrder->Direction);
		m_logger->flush();
		this->firsttime_tocsv++;
		qryOrder_to_csv(pOrder);

	}
	else {
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << " =====�����ѯ����ʧ��=====" << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->info("�����ѯ����ʧ��");
		m_logger->flush();
	}

	if (bIsLast == true) {
		this->taskdone = true;
	}
}


void CustomTradeSpi::OnRspOrderInsert(
	CThostFtdcInputOrderField* pInputOrder,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		std::cout << "=====����¼��ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pInputOrder->InstrumentID << std::endl;
		std::cout << "�۸� " << pInputOrder->LimitPrice << std::endl;
		std::cout << "������ " << pInputOrder->VolumeTotalOriginal << std::endl;
		std::cout << "���ַ��� " << pInputOrder->Direction << std::endl;
	}
}

void CustomTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField* pInputOrderAction,
	CThostFtdcRspInfoField* pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo))
	{
		std::unique_lock<std::mutex> unique(mt);
		std::cout << "=====���������ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pInputOrderAction->InstrumentID << std::endl;
		std::cout << "������־�� " << pInputOrderAction->ActionFlag;
	}
}

void CustomTradeSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	std::unique_lock<std::mutex> unique(mt);
	char str[10];
	sprintf(str, "%d", pOrder->OrderSubmitStatus);
	int orderState = atoi(str) - 48;	//����״̬0=�Ѿ��ύ��3=�Ѿ�����

	//std::cout << "=====�յ�����Ӧ��=====" << std::endl;

	if (isMyOrder(pOrder))
	{
		if (isTradingOrder(pOrder))
		{
			std::cout << "--->>> �ȴ��ɽ��У�" << std::endl;
			//reqOrderAction(pOrder); // ������Գ���
			//reqUserLogout(); // �ǳ�����
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
			std::cout << "--->>> �����ɹ���" << std::endl;
	}
}

void CustomTradeSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	std::unique_lock<std::mutex> unique(mt);
	std::cout << "=====�����ɹ��ɽ�=====" << std::endl;
	std::cout << "�ɽ�ʱ�䣺 " << pTrade->TradeTime << std::endl;
	std::cout << "��Լ���룺 " << pTrade->InstrumentID << std::endl;
	std::cout << "�ɽ��۸� " << pTrade->Price << std::endl;
	std::cout << "�ɽ����� " << pTrade->Volume << std::endl;
	std::cout << "��ƽ�ַ��� " << pTrade->Direction << std::endl;
}

bool CustomTradeSpi::isErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	std::unique_lock<std::mutex> unique(mt);
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult) {
		std::cout << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
		this->taskdone = true;

		m_logger->error("���ش���,ErrorID={},ErrorMsg={}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		m_logger->flush();
	}

	return bResult;
}

void CustomTradeSpi::reqUserLogin()
{
	std::unique_lock<std::mutex> unique(mt);

	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, gBrokerID);
	strcpy(loginReq.UserID, gInvesterID);
	strcpy(loginReq.Password, gInvesterPassword);
	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
	{
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << ">>>>>>���͵�¼����ɹ�" << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->info("���͵�¼����ɹ�");
		m_logger->flush();

	}
	else
	{
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << "--->>>���͵�¼����ʧ��" << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->error("���͵�¼����ʧ��");
		m_logger->flush();
	}


}

void CustomTradeSpi::reqUserLogout()
{
	CThostFtdcUserLogoutField logoutReq;
	memset(&logoutReq, 0, sizeof(logoutReq));
	strcpy(logoutReq.BrokerID, gBrokerID);
	strcpy(logoutReq.UserID, gInvesterID);
	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqUserLogout(&logoutReq, requestID);
	if (!rt)
		std::cout << ">>>>>>���͵ǳ�����ɹ�" << std::endl;
	else
		std::cerr << "--->>>���͵ǳ�����ʧ��" << std::endl;
}


void CustomTradeSpi::reqSettlementInfoConfirm()
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcSettlementInfoConfirmField settlementConfirmReq;
	memset(&settlementConfirmReq, 0, sizeof(settlementConfirmReq));
	strcpy(settlementConfirmReq.BrokerID, gBrokerID);
	strcpy(settlementConfirmReq.InvestorID, gInvesterID);
	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqSettlementInfoConfirm(&settlementConfirmReq, requestID);
	if (!rt)
		std::cout << ">>>>>>����Ͷ���߽�����ȷ������ɹ�" << std::endl;
	else
		std::cerr << "--->>>����Ͷ���߽�����ȷ������ʧ��" << std::endl;
}

void CustomTradeSpi::reqQueryInstrument()
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcQryInstrumentField instrumentReq;
	memset(&instrumentReq, 0, sizeof(instrumentReq));
	strcpy(instrumentReq.InstrumentID, g_pTradeInstrumentID);
	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqQryInstrument(&instrumentReq, requestID);
	if (!rt)
		std::cout << ">>>>>>���ͺ�Լ��ѯ����ɹ�" << std::endl;
	else
		std::cerr << "--->>>���ͺ�Լ��ѯ����ʧ��" << std::endl;
}

void CustomTradeSpi::reqQueryTradingAccount()
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcQryTradingAccountField tradingAccountReq;
	memset(&tradingAccountReq, 0, sizeof(tradingAccountReq));
	strcpy(tradingAccountReq.BrokerID, gBrokerID);
	strcpy(tradingAccountReq.InvestorID, gInvesterID);
	static int requestID = 0; // ������
	std::this_thread::sleep_for(std::chrono::milliseconds(1700)); // ��ʱ����Ҫͣ��һ����ܲ�ѯ�ɹ�

	int rt = g_pTradeUserApi->ReqQryTradingAccount(&tradingAccountReq, requestID);
	if (!rt)
		std::cout << ">>>>>>����Ͷ�����ʽ��˻���ѯ����ɹ�" << std::endl;
	else
		std::cerr << "--->>>����Ͷ�����ʽ��˻���ѯ����ʧ��" << std::endl;
}

void CustomTradeSpi::reqQueryInvestorPosition()
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcQryInvestorPositionField postionReq;
	memset(&postionReq, 0, sizeof(postionReq));
	strcpy(postionReq.BrokerID, gBrokerID);
	strcpy(postionReq.InvestorID, gInvesterID);
	strcpy(postionReq.InstrumentID, g_pTradeInstrumentID);
	static int requestID = 0; // ������

	std::this_thread::sleep_for(std::chrono::milliseconds(1700)); // ��ʱ����Ҫͣ��һ����ܲ�ѯ�ɹ�

	int rt = g_pTradeUserApi->ReqQryInvestorPosition(&postionReq, requestID);
	if (!rt)
		std::cout << ">>>>>>����Ͷ���ֲֲ߳�ѯ����ɹ�" << std::endl;
	else
		std::cerr << "--->>>����Ͷ���ֲֲ߳�ѯ����ʧ��" << std::endl;
}

void CustomTradeSpi::reqQryOrder() // �����ѯ����
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcQryOrderField pQryOrder;
	memset(&pQryOrder, 0, sizeof(pQryOrder));
	strcpy(pQryOrder.BrokerID, gBrokerID);
	strcpy(pQryOrder.InvestorID, gInvesterID);
	//strcpy(pQryOrder.InstrumentID, g_pTradeInstrumentID);
	static int requestID = 0; // ������

	std::this_thread::sleep_for(std::chrono::milliseconds(1700)); // ��ʱ����Ҫͣ��һ����ܲ�ѯ�ɹ�

	int rt = g_pTradeUserApi->ReqQryOrder(&pQryOrder, requestID);
	if (!rt)
	{
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << ">>>>>>���������ѯ�����ɹ�" << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->info("���������ѯ�����ɹ�");
		m_logger->flush();
	}

	else
	{
		std::cout << "------------------------------------------------" << std::endl;
		std::cerr << "--->>>���������ѯ����ʧ��" << std::endl;
		std::cout << "------------------------------------------------" << std::endl;

		m_logger->info("���������ѯ����ʧ��");
		m_logger->flush();
	}

}
void CustomTradeSpi::reqOrderInsert()
{
	std::unique_lock<std::mutex> unique(mt);
	CThostFtdcInputOrderField orderInsertReq;
	memset(&orderInsertReq, 0, sizeof(orderInsertReq));
	///���͹�˾����
	strcpy(orderInsertReq.BrokerID, gBrokerID);
	///Ͷ���ߴ���
	strcpy(orderInsertReq.InvestorID, gInvesterID);
	///��Լ����
	strcpy(orderInsertReq.InstrumentID, g_pTradeInstrumentID);
	///��������
	strcpy(orderInsertReq.OrderRef, order_ref);
	///�����۸�����: �޼�
	orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///��������: 
	//orderInsertReq.Direction = gTradeDirection;
	orderInsertReq.Direction = THOST_FTDC_D_Buy;
	///��Ͽ�ƽ��־: ����
	orderInsertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///���Ͷ���ױ���־
	orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///�۸�
	orderInsertReq.LimitPrice = gLimitPrice;
	///������1
	orderInsertReq.VolumeTotalOriginal = 1;
	///��Ч������: ������Ч
	orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
	///�ɽ�������: �κ�����
	orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	orderInsertReq.MinVolume = 1;
	///��������: ����
	orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ǿƽԭ��: ��ǿƽ
	orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	orderInsertReq.IsAutoSuspend = 0;
	///�û�ǿ����־: ��
	orderInsertReq.UserForceClose = 0;

	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqOrderInsert(&orderInsertReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>���ͱ���¼������ɹ�" << std::endl;
	else
		std::cerr << "--->>>���ͱ���¼������ʧ��" << std::endl;
}

void CustomTradeSpi::reqOrderInsert(
	TThostFtdcInstrumentIDType instrumentID,
	TThostFtdcPriceType price,
	TThostFtdcVolumeType volume,
	TThostFtdcDirectionType direction)
{
	CThostFtdcInputOrderField orderInsertReq;
	memset(&orderInsertReq, 0, sizeof(orderInsertReq));
	///���͹�˾����
	strcpy(orderInsertReq.BrokerID, gBrokerID);
	///Ͷ���ߴ���
	strcpy(orderInsertReq.InvestorID, gInvesterID);
	///��Լ����
	strcpy(orderInsertReq.InstrumentID, instrumentID);
	///��������
	strcpy(orderInsertReq.OrderRef, order_ref);
	///�����۸�����: �޼�
	orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	///��������: 
	orderInsertReq.Direction = direction;
	///��Ͽ�ƽ��־: ����
	orderInsertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	///���Ͷ���ױ���־
	orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///�۸�
	orderInsertReq.LimitPrice = price;
	///������1
	orderInsertReq.VolumeTotalOriginal = volume;
	///��Ч������: ������Ч
	orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
	///�ɽ�������: �κ�����
	orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	orderInsertReq.MinVolume = 1;
	///��������: ����
	orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ǿƽԭ��: ��ǿƽ
	orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	orderInsertReq.IsAutoSuspend = 0;
	///�û�ǿ����־: ��
	orderInsertReq.UserForceClose = 0;

	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqOrderInsert(&orderInsertReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>���ͱ���¼������ɹ�" << std::endl;
	else
		std::cerr << "--->>>���ͱ���¼������ʧ��" << std::endl;
}

void CustomTradeSpi::reqOrderAction(CThostFtdcOrderField* pOrder)
{
	static bool orderActionSentFlag = false; // �Ƿ����˱���
	if (orderActionSentFlag)
		return;

	CThostFtdcInputOrderActionField orderActionReq;
	memset(&orderActionReq, 0, sizeof(orderActionReq));
	///���͹�˾����
	strcpy(orderActionReq.BrokerID, pOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(orderActionReq.InvestorID, pOrder->InvestorID);
	///������������
	//	TThostFtdcOrderActionRefType	OrderActionRef;
	///��������
	strcpy(orderActionReq.OrderRef, pOrder->OrderRef);
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	orderActionReq.FrontID = trade_front_id;
	///�Ự���
	orderActionReq.SessionID = session_id;
	///����������
	//	TThostFtdcExchangeIDType	ExchangeID;
	///�������
	//	TThostFtdcOrderSysIDType	OrderSysID;
	///������־
	orderActionReq.ActionFlag = THOST_FTDC_AF_Delete;
	///�۸�
	//	TThostFtdcPriceType	LimitPrice;
	///�����仯
	//	TThostFtdcVolumeType	VolumeChange;
	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(orderActionReq.InstrumentID, pOrder->InstrumentID);
	static int requestID = 0; // ������
	int rt = g_pTradeUserApi->ReqOrderAction(&orderActionReq, ++requestID);
	if (!rt)
		std::cout << ">>>>>>���ͱ�����������ɹ�" << std::endl;
	else
		std::cerr << "--->>>���ͱ�����������ʧ��" << std::endl;
	orderActionSentFlag = true;
}

bool CustomTradeSpi::isMyOrder(CThostFtdcOrderField* pOrder)
{
	return ((pOrder->FrontID == trade_front_id) &&
		(pOrder->SessionID == session_id) &&
		(strcmp(pOrder->OrderRef, order_ref) == 0));
}

bool CustomTradeSpi::isTradingOrder(CThostFtdcOrderField* pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

//��ѯ�������¼����csv�ļ�

bool CustomTradeSpi::isFileExists_ifstream(std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}
void CustomTradeSpi::qryOrder_to_csv(CThostFtdcOrderField* pOrder)
{


	struct tm* tm;//��ֵ�ṹ������ tm
	time_t t;//�õ���1970��1��1�յ�Ŀǰ������ t
	t = time(NULL);
	tm = localtime(&t);

	std::string year = std::to_string(tm->tm_year + 1900);
	//std::string mon = std::to_string(tm->tm_mon + 1);
	std::string day = std::to_string(tm->tm_mday);
	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << tm->tm_mon + 1;
	std::string mon = ss.str();

	std::string mydir = year + mon + day;

	int i = _mkdir(mydir.c_str());//����Ŀ¼,���û���򴴽�




	std::ofstream outFile;
	std::string filename = gInvesterID;
	filename += ".csv";
	filename = mydir + "/" + filename;

	if (isFileExists_ifstream(filename)&&this->firsttime_tocsv==1)
	{
		remove(filename.c_str());//����ļ�������ɾ��
	}

	outFile.open(filename, std::ios::app);


	// д�������
	if (this->firsttime_tocsv == 1) {

		outFile << "BrokerID" << ','
			<< "InvestorID" << ','
			<< "InstrumentID" << ','
			<< "OrderRef" << ','
			<< "UserID" << ','
			<< "OrderPriceType" << ','
			<< "Direction" << ','
			<< "CombOffsetFlag" << ','
			<< "CombHedgeFlag" << ','
			<< "LimitPrice" << ','
			<< "VolumeTotalOriginal" << ','
			<< "TimeCondition" << ','
			<< "GTDDate" << ','
			<< "VolumeCondition" << ','
			<< "MinVolume" << ','
			<< "ContingentCondition" << ','
			<< "StopPrice" << ','
			<< "ForceCloseReason" << ','
			<< "IsAutoSuspend" << ','
			<< "BusinessUnit" << ','
			<< "RequestID" << ','
			<< "OrderLocalID" << ','
			<< "ExchangeID" << ','
			<< "ParticipantID" << ','
			<< "ClientID" << ','
			<< "ExchangeInstID" << ','
			<< "TraderID" << ','
			<< "InstallID" << ','
			<< "OrderSubmitStatus" << ','
			<< "NotifySequence" << ','
			<< "TradingDay" << ','
			<< "SettlementID" << ','
			<< "OrderSysID" << ','
			<< "OrderSource" << ','
			<< "OrderStatus" << ','
			<< "OrderType" << ','
			<< "VolumeTraded" << ','
			<< "VolumeTotal" << ','
			<< "InsertDate" << ','
			<< "InsertTime" << ','
			<< "ActiveTime" << ','
			<< "SuspendTime" << ','
			<< "UpdateTime" << ','
			<< "CancelTime" << ','
			<< "ActiveTraderID" << ','
			<< "ClearingPartID" << ','
			<< "SequenceNo" << ','
			<< "FrontID" << ','
			<< "SessionID" << ','
			<< "UserProductInfo" << ','
			<< "StatusMsg" << ','
			<< "UserForceClose" << ','
			<< "ActiveUserID" << ','
			<< "BrokerOrderSeq" << ','
			<< "RelativeOrderSysID" << ','
			<< "ZCETotalTradedVolume" << ','
			<< "IsSwapOrder" << ','
			<< "BranchID" << ','
			<< "InvestUnitID" << ','
			<< "AccountID" << ','
			<< "CurrencyID" << ','
			<< "IPAddress" << ','
			<< "MacAddress" << std::endl;
	}

	
	
	//
	//// ********д������*********
	// д���ַ���(����)
	outFile << pOrder->BrokerID << ','
		<< pOrder->InvestorID << ','
		<< pOrder->InstrumentID << ','
		<< pOrder->OrderRef << ','
		<< pOrder->UserID << ','
		<< pOrder->OrderPriceType << ','
		<< pOrder->Direction << ','
		<< pOrder->CombOffsetFlag << ','
		<< pOrder->CombHedgeFlag << ','
		<< pOrder->LimitPrice << ','
		<< pOrder->VolumeTotalOriginal << ','
		<< pOrder->TimeCondition << ','
		<< pOrder->GTDDate << ','
		<< pOrder->VolumeCondition << ','
		<< pOrder->MinVolume << ','
		<< pOrder->ContingentCondition << ','
		<< pOrder->StopPrice << ','
		<< pOrder->ForceCloseReason << ','
		<< pOrder->IsAutoSuspend << ','
		<< pOrder->BusinessUnit << ','
		<< pOrder->RequestID << ','
		<< pOrder->OrderLocalID << ','
		<< pOrder->ExchangeID << ','
		<< pOrder->ParticipantID << ','
		<< pOrder->ClientID << ','
		<< pOrder->ExchangeInstID << ','
		<< pOrder->TraderID << ','
		<< pOrder->InstallID << ','
		<< pOrder->OrderSubmitStatus << ','
		<< pOrder->NotifySequence << ','
		<< pOrder->TradingDay << ','
		<< pOrder->SettlementID << ','
		<< pOrder->OrderSysID << ','
		<< pOrder->OrderSource << ','
		<< pOrder->OrderStatus << ','
		<< pOrder->OrderType << ','
		<< pOrder->VolumeTraded << ','
		<< pOrder->VolumeTotal << ','
		<< pOrder->InsertDate << ','
		<< pOrder->InsertTime << ','
		<< pOrder->ActiveTime << ','
		<< pOrder->SuspendTime << ','
		<< pOrder->UpdateTime << ','
		<< pOrder->CancelTime << ','
		<< pOrder->ActiveTraderID << ','
		<< pOrder->ClearingPartID << ','
		<< pOrder->SequenceNo << ','
		<< pOrder->FrontID << ','
		<< pOrder->SessionID << ','
		<< pOrder->UserProductInfo << ','
		<< pOrder->StatusMsg << ','
		<< pOrder->UserForceClose << ','
		<< pOrder->ActiveUserID << ','
		<< pOrder->BrokerOrderSeq << ','
		<< pOrder->RelativeOrderSysID << ','
		<< pOrder->ZCETotalTradedVolume << ','
		<< pOrder->IsSwapOrder << ','
		<< pOrder->BranchID << ','
		<< pOrder->InvestUnitID << ','
		<< pOrder->AccountID << ','
		<< pOrder->CurrencyID << ','
		<< pOrder->IPAddress << ','
		<< pOrder->MacAddress << std::endl;


	outFile.close();
	
	std::cout << "------------------------------------------------" << std::endl;
	std::cout << "Write to " << filename << ".csv is done!!!" << std::endl;
	std::cout << "------------------------------------------------" << std::endl;


	alerttoWechat(pOrder, this->ContractAlertNum, this->AllContractAlertNum);

	m_logger->info("Write to {}.csv",filename);
	m_logger->flush();


}

std::string CustomTradeSpi::getHead(const std::string& str)
{
	char c;
	c = str[1];//��ȡ�ڶ����ַ���
	if (c >= '0' && c <= '9')
	{
		//printf("������\n");
		return str.substr(0, 1);
	}
	else  if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
	{
		//printf("����ĸ\n");
		return str.substr(0, 2);
	}

	else
	{	
		
		printf("�������ַ�\n");
		return str;
	}
}

void CustomTradeSpi::alerttoWechat(CThostFtdcOrderField* pOrder,int smallAlertNum,int totalAlertNum)
{
	std::string contract = getHead(pOrder->InstrumentID);
	int status = pOrder->OrderStatus;
	

	if (status == 53)//�ѳ���
	{

		m_ContractInstmap[contract].push_back(std::make_pair(pOrder->InstrumentID, 2));
		m_ContractNummap[contract] += 2;

		if ((m_ContractNummap[contract] % smallAlertNum == 0) || ((m_ContractNummap[contract] - 1) % smallAlertNum == 0))
		{	//baojing
			m_logger->info("contract:{} has achieved {},(order canceled)", contract, m_ContractNummap[contract]);
			m_logger->flush();

			std::string msgcontent = "Account:" + accountname + ":---------" + contract + "has achieved" + std::to_string(m_ContractNummap[contract]);
			sendToWeChat(msgcontent);
			
		}

		totalContractNum += 2;
		if ((totalContractNum % totalAlertNum == 0)|| ((totalContractNum-1)%totalAlertNum==0))
		{	//baojing
			m_logger->info("totalContract has achieved {},(order canceled)", totalContractNum);
			m_logger->flush();

			std::string msgcontent = "Account:" + accountname + ":---------" + "totalcontract" + "has achieved" + std::to_string(totalContractNum);
			sendToWeChat(msgcontent);
		}
	}
	else//�����
	{
		m_ContractInstmap[contract].push_back(std::make_pair(pOrder->InstrumentID, 1));
		m_ContractNummap[contract] += 1;

		if (m_ContractNummap[contract] % smallAlertNum == 0)
		{	//baojing
			m_logger->info("contract:{} has achieved {}", contract, m_ContractNummap[contract]);
			m_logger->flush();

			std::string msgcontent = "Account:" + accountname + ":---------" + contract + "has achieved" + std::to_string(m_ContractNummap[contract]);
			sendToWeChat(msgcontent);
		}

		totalContractNum += 1;
		if (totalContractNum % totalAlertNum == 0)
		{	//baojing
			m_logger->info("totalContract has achieved {}", totalContractNum);
			m_logger->flush();

			std::string msgcontent = "Account:" + accountname + ":---------" + "totalcontract" + "has achieved" + std::to_string(totalContractNum);
			sendToWeChat(msgcontent);
		}
	}
}


void CustomTradespi::sendToWeChat(const std::string& msgcontent)
{
	httplib::Client cli("workwx-notifier.mp.com", 80);
	std::string strJsonContent = "{\"receiver\": \" chatid:wrBKYRDgAALrYsBB6ckHJhh4vLks7Iqg\",\
		\"level\": \"ALERT\",\"code\": \"0\",\"msg\": \"";
	strJsonContent = strJsonContent + msgcontent + "\"}";
	const char* ss = strJsonContent.c_str();
	std::string type = "m_type";
	
	if (auto res = cli.Post("/api/v1/alert", ss, strlen(ss), type))
	{
		m_logger->info("WECHAT MESSAGE RETURN : ALREADY ACCEPT");

		m_logger->flush();
	}
	else
	{
		auto err = res.error();	
		m_logger->error("WECHAT MESSAGE RETURN : ERROR OCCUR");

		m_logger->flush();
		
	}


}






