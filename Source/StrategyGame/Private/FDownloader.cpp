// Fill out your copyright notice in the Description page of Project Settings.

#include "StrategyGame.h"
#include "FDownloader.h"

//FDownloader::FDownloader()
//{
//}
//
//FDownloader::~FDownloader()
//{
//}


#define VERBOSE_HTTP 1		//Lots of output
#define CAN_TO_FILE 0		//Write the Canonical signiture to a file in your project directory
#define STS_TO_FILE 0		//Write the String to Sign to a file in your project directory
#define REPONSE_TO_FILE 1	//Write any HTTP response to file

#if VERBOSE_HTTP
#define HttpOutputf(fmt, ...) UE_LOG(LogTemp, Display, fmt, ##__VA_ARGS__)
#define HttpWarningf(fmt, ...) UE_LOG(LogTemp, Warning, fmt, ##__VA_ARGS__)
#define HttpErrorf(fmt, ...) UE_LOG(LogTemp, Error, fmt, ##__VA_ARGS__)
#else
#define HttpOutputf(fmt, ...)
#define HttpWarningf(fmt, ...) UE_LOG(LogTemp, Warning, fmt, ##__VA_ARGS__)
#define HttpErrorf(fmt, ...) UE_LOG(LogTemp, Error, fmt, ##__VA_ARGS__)
#endif


FDownloader* FDownloader::smp_Instance = NULL;

struct HTTPRequestDetails
{
	FString m_KeyID;
	FString m_SecretKey;
	FString m_InputName;
	FString m_URL;
	FString m_Bucket;
	FString m_Host;
	FString m_Service;
	FString m_Region;
	FString m_SignedHeaders;
	FDateTime date = FDateTime::UtcNow();
	FString m_DayOfYear = FString::Printf(_T("%04d%02d%02d"), date.GetYear(), date.GetMonth(), date.GetDay());
	FString m_TimeOfDay = FString::Printf(_T("%02d%02d%02d"), date.GetHour(), date.GetMinute(), date.GetSecond());
	FString m_DateString = FString::Printf(_T("%sT%sZ"), *m_DayOfYear, *m_TimeOfDay);

	HTTPRequestDetails( const FString &rBucket, const FString &rInputName)
	{
		m_InputName = rInputName;
		m_Service = "s3";
		m_Bucket = rBucket;
		m_Host = FString::Printf(_T("%s.amazonaws.com"), *m_Service);
		m_Region = "us-east-1";
		FDateTime dateTemp = FDateTime::UtcNow();
		m_DayOfYear = FString::Printf(_T("%04d%02d%02d"), dateTemp.GetYear(), dateTemp.GetMonth(), dateTemp.GetDay());
		m_TimeOfDay = FString::Printf(_T("%02d%02d%02d"), dateTemp.GetHour(), dateTemp.GetMinute(), dateTemp.GetSecond());
		m_DateString = FString::Printf(_T("%sT%sZ"), *m_DayOfYear, *m_TimeOfDay);
	}

	void SetupForFile()
	{
		m_URL = FString::Printf(_T("https://%s.amazonaws.com%s%s"), *m_Service, *m_Bucket, *m_InputName);
	}

	void SetupForURL()
	{
		m_URL = FString::Printf(_T("https://%s.amazonaws.com/%s"), *m_Service, *m_InputName);
	}
};

FDownloader& FDownloader::GetInstance()
{
	return *smp_Instance;
}

bool FDownloader::StartService()
{
	m_bHasService = FHttpModule::Get().IsHttpEnabled();
	return m_bHasService;
}
bool FDownloader::StartDownload(const FString &rBucket, const FString &rFile, HttpRequestCompleteFn onCompletion )
{
	HttpOutputf(_T("------------------------------"));
	HttpOutputf(_T("AMZ Start Download"));
	if (!m_bHasService)
	{
		HttpErrorf(_T("AWSS3HttpDownloader::StartDownload: No service!"));
		HttpOutputf(_T("------------------------------"));
		return nullptr;
	}

	auto request = FHttpModule::Get().CreateRequest();
	request->SetVerb("GET");

#if REPONSE_TO_FILE
	auto withFileDump = [onCompletion](FHttpRequestPtr request, FHttpResponsePtr response, bool somebool)
	{
		FFileHelper::SaveStringToFile(response->GetContentAsString(), *(FPaths::GetProjectFilePath() + "HTTPResponse.xml"));
		onCompletion(request, response, somebool);
	};
	request->OnProcessRequestComplete().BindLambda(withFileDump);
#else
	request->OnProcessRequestComplete().BindLambda(onCompletion);
#endif

	HTTPRequestDetails details( rBucket, rFile);
	details.SetupForFile();
	SetupAWSCore(details, request);

	HttpOutputf(_T("------------------------------"));
	HttpOutputf(_T("INPUTS"));
	HttpOutputf(_T("\trInput: %s"), *details.m_InputName);
	HttpOutputf(_T("\trHost: %s"), *details.m_Host);
	HttpOutputf(_T("\trVerb: %s"), *request->GetVerb());
	HttpOutputf(_T("\tURL: %s"), *details.m_URL);
	HttpOutputf(_T("\tDayOfYear: %s"), *details.m_DayOfYear);
	HttpOutputf(_T("\tTimeOfDay: %s"), *details.m_TimeOfDay);
	HttpOutputf(_T("\tDateString: %s"), *details.m_DateString);
	HttpOutputf(_T("\tService: %s"), *details.m_Service);
	HttpOutputf(_T("\tRegion: %s"), *details.m_Region);


	//Output only
	HttpOutputf(_T("------------------------------"));
	auto headerList = request->GetAllHeaders();
	HttpOutputf(_T("AMZ: Full header list [%d elements]"), headerList.Num());
	for (auto &rHdr : headerList)
	{
		HttpOutputf(_T("\t%s"), *rHdr);
	}
	HttpOutputf(_T("------------------------------"));
	HttpOutputf(_T("\n"));

	return request->ProcessRequest();
}

void FDownloader::SetupAWSCore(struct HTTPRequestDetails& rReqDetails, TSharedPtr<IHttpRequest> request)
{
	request->SetURL(rReqDetails.m_URL);
	request->SetHeader("host", rReqDetails.m_Host);
	request->SetHeader("x-amz-date", *rReqDetails.m_DateString);
}