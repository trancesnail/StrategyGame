// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Http.h"
/**
 * 
 */

typedef TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> HttpRequestCompleteFn;

class STRATEGYGAME_API FDownloader
{
//public:
	//FDownloader();
	//~FDownloader();

	bool m_bHasService = false;
	static FDownloader *smp_Instance;
public:
	static FDownloader &GetInstance();
	//{
	//	return *smp_Instance;
	//}
	bool StartService();
	bool StartDownload(const FString &rBucket, const FString &rFile, HttpRequestCompleteFn onCompletion = HttpRequestCompleteFn());

private:
	void SetupAWSCore(struct HTTPRequestDetails& rReqDetails, TSharedPtr<IHttpRequest> request);

};
