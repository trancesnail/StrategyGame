// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "StrategyGame.h"
#include "StrategyMenuHUD.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "SStrategyMenuWidget.h"
#include "StrategyMenuItem.h"
#include "StrategyHelpers.h"
#include "StrategyGameLoadingScreen.h"
#include "StrategyHUDSoundsWidgetStyle.h"


#include "zlib.h"

#define LOCTEXT_NAMESPACE "StrategyGame.HUD.Menu"

AStrategyMenuHUD::AStrategyMenuHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MenuButtonTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/MainMenu/MenuButton.MenuButton"), nullptr, LOAD_None, nullptr);

	AddMenuItem(MainMenu,LOCTEXT("Start", "START"));
	AddMenuItem(MainMenu->Last().SubMenu,LOCTEXT("Easy", "EASY"))->OnConfirmMenuItem.BindUObject(this,&AStrategyMenuHUD::ExecuteSelectMapAction,(int32)EMenuAction::SetEasy);
	AddMenuItem(MainMenu->Last().SubMenu,LOCTEXT("Normal", "NORMAL"))->OnConfirmMenuItem.BindUObject(this,&AStrategyMenuHUD::ExecuteSelectMapAction,(int32)EMenuAction::SetMedium);
	AddMenuItem(MainMenu->Last().SubMenu,LOCTEXT("Hard", "HARD"))->OnConfirmMenuItem.BindUObject(this,&AStrategyMenuHUD::ExecuteSelectMapAction,(int32)EMenuAction::SetHard);
	AddMenuItem(MainMenu->Last().SubMenu,LOCTEXT("Back", "BACK"))->OnConfirmMenuItem.BindUObject(this,&AStrategyMenuHUD::ExecuteSelectMapAction,(int32)EMenuAction::GoBack);

	if (FPlatformProperties::SupportsQuit())
	{
		AddMenuItem(MainMenu, LOCTEXT("Quit", "QUIT"))->OnConfirmMenuItem.BindUObject(this, &AStrategyMenuHUD::ExecuteQuitAction);
	}
	CurrentMenu = MainMenu;

	Http = &FHttpModule::Get();

}

FStrategyMenuItem* AStrategyMenuHUD::AddMenuItem(TSharedPtr<TArray<class FStrategyMenuItem>> &SubMenu, FText Text)
{
	if (!SubMenu.IsValid())
	{
		SubMenu = MakeShareable(new TArray<FStrategyMenuItem>());
	}
	FStrategyMenuItem MenuItem(Text);
	SubMenu->Add(MenuItem);
	return &SubMenu->Last();
}

void AStrategyMenuHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	//Now that we are here, build our menu widget
	RebuildWidgets();
}

void AStrategyMenuHUD::ExecuteQuitAction()
{
	MenuWidget->HideMenu();
	const FStrategyHUDSoundsStyle& HUDSounds = FStrategyStyle::Get().GetWidgetStyle<FStrategyHUDSoundsStyle>("DefaultStrategyHUDSoundsStyle");
	MenuHelper::PlaySoundAndCall(PlayerOwner->GetWorld(),HUDSounds.ExitGameSound,this, &AStrategyMenuHUD::Quit);
	MenuWidget->LockControls(true);
}

void AStrategyMenuHUD::Quit()
{
	GetOwningPlayerController()->ConsoleCommand("quit");
}

void AStrategyMenuHUD::ExecuteSelectMapAction(int32 index)
{
	switch (index)
	{
		case EMenuAction::GoBack:
			MenuWidget->MenuGoBack();
			MyHttpCall();
			return;
		case EMenuAction::SetEasy:
			Difficulty = EGameDifficulty::Easy;
			break;
		case EMenuAction::SetMedium:
			Difficulty = EGameDifficulty::Medium;
			break;
		case EMenuAction::SetHard:
			Difficulty = EGameDifficulty::Hard;
			break;
		default:
			return;
	}

	MenuWidget->HideMenu();
	MenuWidget->LockControls(true);
	const FStrategyHUDSoundsStyle& HUDSounds = FStrategyStyle::Get().GetWidgetStyle<FStrategyHUDSoundsStyle>("DefaultStrategyHUDSoundsStyle");
	MenuHelper::PlaySoundAndCall(PlayerOwner->GetWorld(),HUDSounds.StartGameSound,this,&AStrategyMenuHUD::LaunchGame);
}

void AStrategyMenuHUD::LaunchGame()
{
	FString StartStr = FString::Printf(TEXT("/Game/Maps/TowerDefenseMap?%s=%d"), *AStrategyGameMode::DifficultyOptionName, (uint8) Difficulty);
	UE_LOG(LogGame, Error, TEXT("AStrategyMenuHUD::%s:StartStr:%s"), __FUNCTION__, *StartStr);
	GetWorld()->ServerTravel(StartStr);
	ShowLoadingScreen();
}

void AStrategyMenuHUD::RebuildWidgets(bool bHotReload)
{
	MenuWidget.Reset();

	if (GEngine && 
		GEngine->GameViewport)
	{
		UGameViewportClient* GVC = GEngine->GameViewport;
		SAssignNew(MenuWidget, SStrategyMenuWidget)
			.Cursor(EMouseCursor::Default)
			.MenuHUD(TWeakObjectPtr<AStrategyMenuHUD>(this));

		GVC->AddViewportWidgetContent(
			SNew(SWeakWidget)
			.PossiblyNullContent(MenuWidget.ToSharedRef()));
	}
}

void AStrategyMenuHUD::ShowLoadingScreen()
{
	IStrategyGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IStrategyGameLoadingScreenModule>("StrategyGameLoadingScreen");
	if( LoadingScreenModule != nullptr )
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void AStrategyMenuHUD::MyHttpCall()
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AStrategyMenuHUD::OnResponseReceived);
	Request->OnRequestProgress().BindUObject(this,&AStrategyMenuHUD::RequestProgress);
	//This is the url on which to process the request
	Request->SetURL("http://xindong.cdnunion.com/app/config_zh_CN.zip");
	Request->SetVerb("GET");
	//Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	//Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->ProcessRequest();
	UE_LOG(LogGame, Error, TEXT("AStrategyMenuHUD::MyHttpCall()"));
	
	FDataBaseConnection * con =  FDataBaseConnection::CreateObject();
	if (con == nullptr ) return;
	if (con->Open(TEXT("Provider=SQLOLEDB.1;Password=123!@#qwe;Persist Security Info=True;User ID=sa;Initial Catalog=Test;Data Source=WIN-KV01ERT77FF\\COLLEGE"), TEXT("192.168.0.253"), TEXT("Provider=SQLOLEDB.1;Password=123!@#qwe;Persist Security Info=True;User ID=sa;Initial Catalog=Test;Data Source=WIN-KV01ERT77FF\\COLLEGE")) )
	{
		UE_LOG(LogGame, Error, TEXT("FDataBaseConnection::Open() True!"));
		FDataBaseRecordSet * pSet ;
		if (con->Execute(TEXT("SELECT * FROM [Test].[dbo].[Player] WHERE [id] >= 30162 and [id] <= 30170"), pSet))
		{
			UE_LOG(LogGame, Error, TEXT("FDataBaseConnection::Execute() True!"));
			FDataBaseRecordSet::TIterator It(pSet);
			while (It)
			{
				UE_LOG(LogGame, Error, TEXT("%ld"), pSet->GetBigInt(TEXT("id")));
				It.operator ++();
			}
			//for (FDataBaseRecordSet::TIterator It(&RecordSet); It; It++)
			//{
			//	UE_LOG(LogGame, Error, TEXT("%ld"),RecordSet.GetBigInt(TEXT("id")) );
			//}
			delete(pSet);
		}
		con->Close();
	}
	else
	{
		UE_LOG(LogGame, Error, TEXT("FDataBaseConnection::Open() Fail!"));
	}
	
}

void AStrategyMenuHUD::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	//Create a pointer to hold the json serialized data

	if (Response.Get() == nullptr)
	{
		return;
	}

	UE_LOG(LogGame, Error, TEXT("AStrategyMenuHUD::OnResponseReceived()"));
	FString msg = Response->GetContentAsString();


	const TArray<uint8>& data = Response->GetContent();
	FString GameContentDir = FPaths::GameContentDir();
	UE_LOG(LogGame, Error, TEXT("GameContentDir：%s"), *GameContentDir);
	FString saveData = GameContentDir + TEXT("/Data/MyData.zip");
	FFileHelper::SaveArrayToFile(data, *saveData);

	UE_LOG(LogGame, Error, TEXT("%s"),*msg);
	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, msg);


	int32/*uLongf*/ slen = data.Num();
	ULONG64 upband = compressBound(slen);

	int32/*uLongf*/ dlen = upband ;
	Bytef * des  =new Bytef[dlen] ;

	TArray<uint8> DestData;
	DestData.Reset(dlen);
	DestData.AddUninitialized(dlen);

	const bool bResult = FCompression::UncompressMemory(COMPRESS_ZLIB, DestData.GetData(), upband, data.GetData(), slen);

	//int result = uncompress(des, &dlen, data.GetData(), slen);

	if (bResult == Z_OK)
	{
		UE_LOG(LogGame, Error, TEXT("解压成功！"));
	}
	else
	{
		UE_LOG(LogGame, Error, TEXT("解压失败！"));
	}

//	TSharedPtr<FJsonObject> JsonObject;
//
//	//Create a reader pointer to read the json data
//	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
//
//	//Deserialize the json data given Reader and the actual object to deserialize
//	if (FJsonSerializer::Deserialize(Reader, JsonObject))
//	{
//		//Get the value of the json object by field name
//		int32 recievedInt = JsonObject->GetIntegerField("customInt");
//
//		//Output it to the engine
//		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, FString::FromInt(recievedInt));
//	}
}

void AStrategyMenuHUD::RequestProgress(FHttpRequestPtr Request, int32 sent, int32 received)
{
	UE_LOG(LogGame, Error, TEXT("AStrategyMenuHUD::RequestProgress():sent:%d---received:%d"), sent, received);
}

#undef LOCTEXT_NAMESPACE