#pragma once
// ---- �����Ľ����� ---- //
#include "CTP_API/ThostFtdcTraderApi.h"
#include <mutex>
#include<unordered_map>
#include"httplib.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

class CustomTradeSpi : public CThostFtdcTraderSpi
{
	// ---- ctp_api���ֻص��ӿ� ---- //
public:

	CustomTradeSpi(const std::string& username,int ContractAlertNum,int AllContractAlertNum);

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	void OnFrontConnected();

	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����Ӧ��
	void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	void OnFrontDisconnected(int nReason);

	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	void OnHeartBeatWarning(int nTimeLapse);

	///�ǳ�������Ӧ
	void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///Ͷ���߽�����ȷ����Ӧ
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ��Ӧ
	void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ʽ��˻���Ӧ
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���Ӧ
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��������������Ӧ
	void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����֪ͨ
	void OnRtnOrder(CThostFtdcOrderField* pOrder);

	///�ɽ�֪ͨ
	void OnRtnTrade(CThostFtdcTradeField* pTrade);

	//�����ѯ������Ӧ
	void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	// ---- �Զ��庯�� ---- //
public:
	bool loginFlag; // ��½�ɹ��ı�ʶ
	void reqOrderInsert(
		TThostFtdcInstrumentIDType instrumentID,
		TThostFtdcPriceType price,
		TThostFtdcVolumeType volume,
		TThostFtdcDirectionType direction); // ���Ի�����¼�룬�ⲿ����
private:
	void reqUserLogin(); // ��¼����
	void reqUserLogout(); // �ǳ�����
	void reqSettlementInfoConfirm(); // Ͷ���߽��ȷ��
	void reqQueryInstrument(); // �����ѯ��Լ
	void reqQueryTradingAccount(); // �����ѯ�ʽ��ʻ�
	void reqQueryInvestorPosition(); // �����ѯͶ���ֲ߳�
	void reqOrderInsert(); // ���󱨵�¼��

	void reqOrderAction(CThostFtdcOrderField* pOrder); // ���󱨵�����
	bool isErrorRspInfo(CThostFtdcRspInfoField* pRspInfo); // �Ƿ��յ�������Ϣ
	bool isMyOrder(CThostFtdcOrderField* pOrder); // �Ƿ��ҵı����ر�
	bool isTradingOrder(CThostFtdcOrderField* pOrder); // �Ƿ����ڽ��׵ı���

	void qryOrder_to_csv(CThostFtdcOrderField* pOrder);//��ѯ�������¼����csv�ļ�
	bool isFileExists_ifstream(std::string& name);//�ж��ļ��Ƿ����
	std::string getHead(const std::string& str);//��ȡĳ��ĵ��ײ�������Լ����

	void reqQryOrder(); // �����ѯ����

	void alerttoWechat(CThostFtdcOrderField* pOrder, int smallAlertNum, int totalAlertNum);//����ҵ΢�ű���

	void sendToWeChat(const std::string& msgcontent)
	std::mutex mt;
public:
	int ContractAlertNum = 0;
	int AllContractAlertNum = 0;
	std::string accountname;
	bool taskdone = false;
	bool islastorder = false;
	int firsttime_tocsv = 0;

	int totalContractNum = 0;
	std::shared_ptr<spdlog::logger> m_logger;

	std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> m_ContractInstmap;//��Լ�����map
	std::unordered_map<std::string, int> m_ContractNummap;//��Լ����map

	std::unordered_map<std::string, int> m_InstNummap;
};
