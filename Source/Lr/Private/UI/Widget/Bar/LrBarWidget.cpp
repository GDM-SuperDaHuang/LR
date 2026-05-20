// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bar/LrBarWidget.h"

#include "Components/ProgressBar.h"
#include "UI/ViewModel/LrMVVMVBar.h"

void ULrBarWidget::SetViewModel(ULrMVVMVBar* InViewModel)
{
	ViewModel = InViewModel;
	Refresh();
}


void ULrBarWidget::Refresh()
{
	if (!ViewModel)
	{
		return;
	}

	MainBar->SetPercent(ViewModel->Percent);
	GhostBar->SetPercent(ViewModel->GhostPercent);

}
