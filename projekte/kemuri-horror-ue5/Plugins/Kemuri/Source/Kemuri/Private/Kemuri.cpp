#include "Kemuri.h"

DEFINE_LOG_CATEGORY(LogKemuri);

void FKemuriModule::StartupModule()
{
	UE_LOG(LogKemuri, Log, TEXT("KEMURI Horror Kit geladen."));
}

void FKemuriModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FKemuriModule, Kemuri)
